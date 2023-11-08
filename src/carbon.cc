/* 
 * Copyright 2023 Daniel Illner <illner.daniel@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
 * */

#include "carbon.h"


char *concat_strs(char *s1, char *s2)
{
  char *con = (char *) malloc(strlen(s1) + strlen(s2) + 1);  
  if (!con) {
    perror("Unable to allocate memory.");
    return NULL;
  }
  strcpy(con, s1);
  strcat(con, s2);
  return con;
}

int get_arg_type(const char* arg) 
{
  if (!strcmp(arg, "-help")) return ARG_HELP;
  if (!strcmp(arg, "-pt"))   return ARG_PT;
  if (!strcmp(arg, "-s"))    return ARG_S;
  if (!strcmp(arg, "-w"))    return ARG_W;
  if (!strcmp(arg, "-h"))    return ARG_H;
  if (!strcmp(arg, "-vfov")) return ARG_VFOV;
  if (!strcmp(arg, "-maxd")) return ARG_MAXD;
  if (!strcmp(arg, "-o"))    return ARG_O;
  if (!strcmp(arg, "-cuda")) return ARG_CUDA;
  return ARG_UNKNOWN;
}

int parse_args(c_state_t *s, int *argc, char ***argv) 
{
  for (int i = 1; i < *argc; ++i) {
    switch (get_arg_type((*argv)[i])) {
      case ARG_HELP:
        return ARG_HELP_R;
      case ARG_PT:
        s->pt= 1;
        s->rt= 0;
        break;
      case ARG_S:
        if (++i >= *argc) goto check_arg_err;
        s->spp = atoi((*argv)[i]);
        break;
      case ARG_O:
        if (++i >= *argc) goto check_arg_err;
        s->outfile = (*argv)[i];
        break;
      case ARG_W:
        if (++i >= *argc) goto check_arg_err;
        s->w = atoi((*argv)[i]);
        break;
      case ARG_H:
        if (++i >= *argc) goto check_arg_err;
        s->h = atoi((*argv)[i]);
        break;
      case ARG_VFOV:
        if (++i >= *argc) goto check_arg_err;
        s->vfov = atoi((*argv)[i]);
        break;
      case ARG_MAXD:
        if (++i >= *argc) goto check_arg_err;
        s->maxd = atoi((*argv)[i]);
        break;
      case ARG_CUDA:
        s->cuda = 1;
        break;
      default:
        fprintf(stderr, "ERROR: unknown option %s\n", (*argv)[i-1]);
        return -1;

    check_arg_err:
        fprintf(stderr, "ERROR: %s requires an argument.\n", (*argv)[i-1]);
        return -1;
    }
  }
  return 0;
}
