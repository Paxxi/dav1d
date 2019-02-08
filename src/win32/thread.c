/*
 * Copyright © 2018, VideoLAN and dav1d authors
 * Copyright © 2018, Two Orioles, LLC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#if defined(_WIN32)

#include <process.h>
#include <stdlib.h>
#include <windows.h>

#include "src/thread.h"

static unsigned __stdcall thread_entrypoint(void *const data) {
    pthread_t *const t = data;
    t->arg = t->func(t->arg);
    return 0;
}

int dav1d_pthread_create(pthread_t *const thread, const void *const attr,
                         void *(*const func)(void*), void *const arg)
{
    thread->func = func;
    thread->arg = arg;
    thread->h = (HANDLE)_beginthreadex(NULL, 0, thread_entrypoint,
                                       thread, 0, NULL);
    return !thread->h;
}

int dav1d_pthread_join(pthread_t *const thread, void **const res) {
    if (WaitForSingleObject(thread->h, INFINITE))
        return 1;

    if (res)
        *res = thread->arg;

    return !CloseHandle(thread->h);
}

int dav1d_pthread_once(pthread_once_t *const once_control,
                       void (*const init_routine)(void))
{
    BOOL pending = FALSE;

    if (InitOnceBeginInitialize(once_control, 0, &pending, NULL) != TRUE)
        return 1;

    if (pending == TRUE)
        init_routine();

    return !InitOnceComplete(once_control, 0, NULL);
}

#endif
