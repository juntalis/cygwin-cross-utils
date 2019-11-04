
add_library(cygshared STATIC shared.h shared.c dynarray.c dynarray.h strbuf.h strbuf.c strarray.h strutil.c strutil.h)
set_target_properties(cygshared PROPERTIES CLEAN_DIRECT_OUTPUT 1)
set_target_properties(cygshared PROPERTIES COMPILE_FLAGS "-fPIC")
