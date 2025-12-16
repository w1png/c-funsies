#ifndef SHARED_H
#define SHARED_H

#define err(msg)                                                               \
  {                                                                            \
    printf("%s:%d (function %s): \"%s\"\n", __FILE__, __LINE__, __func__,      \
           msg);                                                               \
    exit(1);                                                                   \
  }

#define NOT_IMPLEMENTED err("Not implemented")

#endif
