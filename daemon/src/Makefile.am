bin_PROGRAMS = daemon


# set the include path found by configure
INCLUDES= $(all_includes)

# the library search path.
daemon_LDFLAGS = $(all_libraries) 


daemon_SOURCES = daemon.c kern_sock.c pop_ops.c
noinst_HEADERS = kern_sock.h pop_ops.h
