#ifndef MESSAGEQUEUE_H
#define MESSAGEQUEUE_H
#include <mutex>
#include <condition_variable>
#include <list>
#include <queue>
#include "SDL.h"
typedef struct AVMessage {
    int what;
    int arg1;
    int arg2;
    void *obj;
    void (*free_l)(void *obj);
    struct AVMessage *next;
} AVMessage;


class MessageQueue
{
public:
    MessageQueue();

    AVMessage *first_msg, *last_msg;
    int nb_messages;
    int abort_request_;
    std::mutex mutex_;
    std::condition_variable condition_;
    std::queue<AVMessage *> queue_;

    AVMessage *recycle_msg;
    int recycle_count;
    int alloc_count;


    inline void msg_free_res(AVMessage *msg)
    {
        if (!msg || !msg->obj)
            return;
        msg->free_l(msg->obj);
        msg->obj = NULL;
    }

    inline static int msg_queue_put_private(AVMessage *msg)
    {
        AVMessage *msg1;


        msg1 = (AVMessage *)malloc(sizeof(AVMessage));

        if (!msg1)
            return -1;

        *msg1 = *msg;
        std::unique_lock<std::mutex> lock(mutex_);
        queue_.push(msg1);
        condition_.notify_one();

        return 0;
    }


    inline static int msg_queue_put(AVMessage *msg)
    {
        int ret;
        //线程安全保护
        SDL_LockMutex(mutex);
        ret = msg_queue_put_private( msg);
        SDL_UnlockMutex(mutex);

        return ret;
    }

    inline static void msg_init_msg(AVMessage *msg)
    {
        memset(msg, 0, sizeof(AVMessage));
    }

    inline static void msg_queue_put_simple1( int what)
    {
        AVMessage msg;
        msg_init_msg(&msg);
        msg.what = what;
        msg_queue_put( &msg);
    }

    inline static void msg_queue_put_simple2( int what, int arg1)
    {
        AVMessage msg;
        msg_init_msg(&msg);
        msg.what = what;
        msg.arg1 = arg1;
        msg_queue_put( &msg);
    }

    inline static void msg_queue_put_simple3( int what, int arg1, int arg2)
    {
        AVMessage msg;
         //初始化一条空的msg
        msg_init_msg(&msg);
        msg.what = what;
        msg.arg1 = arg1;
        msg.arg2 = arg2;
        //消息入队，这个队列是ffplay->msg_queue。
        msg_queue_put( &msg);
    }

    inline static void msg_obj_free_l(void *obj)
    {
        av_free(obj);
    }

    inline static void msg_queue_put_simple4( int what, int arg1, int arg2, void *obj, int obj_len)
    {
        AVMessage msg;
        msg_init_msg(&msg);
        msg.what = what;
        msg.arg1 = arg1;
        msg.arg2 = arg2;
        msg.obj = av_malloc(obj_len);
        memcpy(msg.obj, obj, obj_len);
        msg.free_l = msg_obj_free_l;
        msg_queue_put( &msg);
    }

    inline static void msg_queue_init(MessageQueue *q)
    {
        memset( 0, sizeof(MessageQueue));
        mutex = SDL_CreateMutex();
        cond = SDL_CreateCond();
        abort_request_ = 1;
    }

    inline static void msg_queue_flush(MessageQueue *q)
    {
        AVMessage *msg, *msg1;

        SDL_LockMutex(mutex);
        for (msg = first_msg; msg != NULL; msg = msg1) {
            msg1 = msg->next;
    #ifdef FFP_MERGE
            av_freep(&msg);
    #else
            msg->next = recycle_msg;
            recycle_msg = msg;
    #endif
        }
        last_msg = NULL;
        first_msg = NULL;
        nb_messages = 0;
        SDL_UnlockMutex(mutex);
    }

    inline static void msg_queue_destroy(MessageQueue *q)
    {
        msg_queue_flush(q);

        SDL_LockMutex(mutex);
        while(recycle_msg) {
            AVMessage *msg = recycle_msg;
            if (msg)
                recycle_msg = msg->next;
            msg_free_res(msg);
            av_freep(&msg);
        }
        SDL_UnlockMutex(mutex);

        SDL_DestroyMutex(mutex);
        SDL_DestroyCond(cond);
    }

    inline static void msg_queue_abort(MessageQueue *q)
    {
        SDL_LockMutex(mutex);

        abort_request_ = 1;

        SDL_CondSignal(cond);

        SDL_UnlockMutex(mutex);
    }

    inline static void msg_queue_start(MessageQueue *q)
    {
        SDL_LockMutex(mutex);
        abort_request_ = 0;

        AVMessage msg;
        msg_init_msg(&msg);
        msg.what = FFP_MSG_FLUSH;
        msg_queue_put_private( &msg);
        SDL_UnlockMutex(mutex);
    }

    /* return < 0 if aborted, 0 if no msg and > 0 if msg.  */
    inline static int msg_queue_get( AVMessage *msg, int block)
    {
        AVMessage *msg1;
        int ret;

        SDL_LockMutex(mutex);

        for (;;) {
            if (abort_request_) {
                ret = -1;
                break;
            }

            msg1 = first_msg;
            if (msg1) {
                first_msg = msg1->next;
                if (!first_msg)
                    last_msg = NULL;
                nb_messages--;
                *msg = *msg1;
                msg1->obj = NULL;
    #ifdef FFP_MERGE
                av_free(msg1);
    #else
                msg1->next = recycle_msg;
                recycle_msg = msg1;
    #endif
                ret = 1;
                break;
            } else if (!block) {
                ret = 0;
                break;
            } else {
                SDL_CondWait(cond, mutex);
            }
        }
        SDL_UnlockMutex(mutex);
        return ret;
    }

    inline static void msg_queue_remove( int what)
    {
        AVMessage **p_msg, *msg, *last_msg;
        SDL_LockMutex(mutex);

        last_msg = first_msg;

        if (!abort_request_ && first_msg) {
            p_msg = &first_msg;
            while (*p_msg) {
                msg = *p_msg;

                if (msg->what == what) {
                    *p_msg = msg->next;
    #ifdef FFP_MERGE
                    av_free(msg);
    #else
                    msg_free_res(msg);
                    msg->next = recycle_msg;
                    recycle_msg = msg;
    #endif
                    nb_messages--;
                } else {
                    last_msg = msg;
                    p_msg = &msg->next;
                }
            }

            if (first_msg) {
                last_msg = last_msg;
            } else {
                last_msg = NULL;
            }
        }

        SDL_UnlockMutex(mutex);
    }
};

#endif // MESSAGEQUEUE_H
