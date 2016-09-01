/*
 * Copyright (c) 2006, Adam Dunkels
 * Copyright (c) 2013, Danyil Bohdan
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the author nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <time.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include "ubasic.h"

static const char program_let[] =
"10 let a = 42\n\
20 stop\n";

static const char program_goto[] =
"10 goto 50\n\
20 goto 40\n\
30 goto 60\n\
40 goto 30\n\
50 goto 20\n\
60 let c = 108\n\
70 stop\n";

static const char program_loop[] =
"10 for i = 0 to 126\n\
20 for j = 0 to 126\n\
30 for k = 0 to 10\n\
40 let a = i * j * k\n\
45 print a, i, j, k\n\
50 next k\n\
60 next j\n\
70 next i\n\
80 stop\n";

static const char program_fibs[] =
"20 let a = 1\n\
40 let b = 1\n\
60 for i = 0 to 8\n\
80 let b = a + b\n\
100 let a = b - a\n\
120 rem print a, b\n\
140 next i\n\
160 stop\n";

static const char program_peek_poke[] =
"10 let a= peek(100) + 20 + 3\n\
20 let z = peek(123)\n\
30 poke 100 - 1, 99\n\
40 poke 0, 0\n\
50 stop\n";

/*---------------------------------------------------------------------------*/
value_t peek(value_t arg) {
    return arg;
}

/*---------------------------------------------------------------------------*/
void poke(value_t arg, value_t value) {
    assert(arg == value);
}

/*---------------------------------------------------------------------------*/
void run(const char program[]) {
  static int test_num = 0;
  clock_t start_t, end_t;
  double delta_t;

  start_t = clock();

  test_num++;
  printf("Running test #%u... ", test_num);
  fflush(stdout);


  ubasic_init_peek_poke(program, &peek, &poke);

  do {
    ubasic_run();
  } while(!ubasic_finished());

  end_t = clock();
  delta_t = (double)(end_t - start_t) / CLOCKS_PER_SEC;

  printf("done. Run time: %.3f s\n", delta_t);
}

/*---------------------------------------------------------------------------*/
int
main(void)
{
  struct typevalue v;
  run(program_let);
  ubasic_get_variable(0, &v, 0, NULL);
  assert(v.d.i == 42 && v.type == TYPE_INTEGER);

  run(program_goto);
  ubasic_get_variable(2, &v, 0, NULL);
  assert(v.d.i == 108 && v.type == TYPE_INTEGER);

  run(program_loop);
  ubasic_get_variable(0, &v, 0, NULL);
  assert(v.d.i == ((value_t)(126 * 126 * 10)) && v.type == TYPE_INTEGER);

  run(program_fibs);
  ubasic_get_variable(1, &v, 0, NULL);
  assert(v.d.i == 89 && v.type == TYPE_INTEGER);

  run(program_peek_poke);
  ubasic_get_variable(0, &v, 0, NULL);
  assert(v.d.i == 123 && v.type == TYPE_INTEGER);
  ubasic_get_variable(25, &v, 0, NULL);
  assert(v.d.i == 123 && v.type == TYPE_INTEGER);

  return 0;
}
/*---------------------------------------------------------------------------*/
