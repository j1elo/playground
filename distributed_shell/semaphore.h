/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  Authors:
 *  2003-12-01: Manuel Freire Moran
 *  prior: Library found in internet...
 */

/*
 * File: semaphore.h
 *
 */

#include <stdio.h>
#include <pthread.h>
#ifndef __SEMAPHORE_H
#define __SEMAPHORE_H

typedef struct Semaphore {
    int v;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} Semaphore;

int semaphore_down (Semaphore * s);
int semaphore_up (Semaphore * s);
void semaphore_destroy (Semaphore * s);
void semaphore_init (Semaphore * s, int v);
int semaphore_value (Semaphore * s);
int semaphore_decrement (Semaphore *s);
int semaphore_wait (Semaphore * s);
int semaphore_up2 (Semaphore * s);

int tw_pthread_cond_signal (pthread_cond_t * c);
int tw_pthread_cond_broadcast (pthread_cond_t * c);
int tw_pthread_cond_wait (pthread_cond_t * c, pthread_mutex_t * m);
int tw_pthread_mutex_unlock (pthread_mutex_t * m);
int tw_pthread_mutex_lock (pthread_mutex_t * m);
void do_error (char *msg);

#endif // __SEMAPHORE_H
