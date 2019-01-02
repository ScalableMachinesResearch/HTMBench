## FFTW can be compiled and subsequently linked against
## various data types.
## There is a single set of include files, and then muttiple libraries,
## One for each type.  I.e. libfftw.a-->double, libfftwf.a-->float

set(FFTW_INC_SEARCHPATH
  /sw/include
  /usr/include
  /usr/local/include
  /usr/include/fftw
  /usr/include/fftw3
  /usr/local/include/fftw
  /usr/local/include/fftw3
)

find_path(FFTW_INCLUDE_PATH fftw3.h ${FFTW_INC_SEARCHPATH})

if(FFTW_INCLUDE_PATH)
  file(TO_CMAKE_PATH "${FFTW_INCLUDE_PATH}" FFTW_INCLUDE_PATH)
  set(FFTW_INCLUDE ${FFTW_INCLUDE_PATH})
endif()

if(FFTW_INCLUDE)
  include_directories(${FFTW_INCLUDE})
endif()

get_filename_component(FFTW_INSTALL_BASE_PATH ${FFTW_INCLUDE_PATH} PATH)

set(FFTW_LIB_SEARCHPATH
  ${FFTW_INSTALL_BASE_PATH}/lib
  ${FFTW_INSTALL_BASE_PATH}/lib64
  /usr/lib/fftw
  /usr/local/lib/fftw
)

if(USE_FFTWD)
  mark_as_advanced(FFTWD_LIB)
  find_library(FFTWD_LIB fftw3 ${FFTW_LIB_SEARCHPATH}) #Double Precision Lib
  find_library(FFTWD_THREADS_LIB fftw3_threads ${FFTW_LIB_SEARCHPATH}) #Double Precision Lib only if compiled with threads support

  if(FFTWD_LIB)
    set(FFTWD_FOUND 1)
    get_filename_component(FFTW_LIBDIR ${FFTWD_LIB} PATH)
    if(FFTWD_THREADS_LIB)
      set(FFTWD_LIB ${FFTWD_LIB} ${FFTWD_THREADS_LIB} )
    endif()
  endif()
endif()

if(USE_FFTWF)
  mark_as_advanced(FFTWF_LIB)
  find_library(FFTWF_LIB fftw3f ${FFTW_LIB_SEARCHPATH}) #Single Precision Lib
  find_library(FFTWF_THREADS_LIB fftw3f_threads ${FFTW_LIB_SEARCHPATH}) #Single Precision Lib only if compiled with threads support

  if(FFTWF_LIB)
    set(FFTWF_FOUND 1)
    get_filename_component(FFTW_LIBDIR ${FFTWF_LIB} PATH)
    if(FFTWF_THREADS_LIB)
      set(FFTWF_LIB ${FFTWF_LIB} ${FFTWF_THREADS_LIB} )
    endif()
  endif()
endif()

set(FFTWD_LIBRARIES ${FFTWD_LIB})
set(FFTWF_LIBRARIES ${FFTWF_LIB})
