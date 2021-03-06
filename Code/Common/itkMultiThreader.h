/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    itkMultiThreader.h
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

  Portions of this code are covered under the VTK copyright.
  See VTKCopyright.txt or http://www.kitware.com/VTKCopyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkMultiThreader_h
#define __itkMultiThreader_h

#include "itkObject.h"
#include "itkMutexLock.h"

#ifdef ITK_USE_PTHREADS
#include <pthread.h>
#endif

namespace itk
{
/** \class MultiThreader
 * \brief A class for performing multithreaded execution
 *
 * Multithreader is a class that provides support for multithreaded
 * execution using sproc() on an SGI, or pthread_create on any platform
 * supporting POSIX threads.  This class can be used to execute a single
 * method on multiple threads, or to specify a method per thread.
 *
 * \ingroup OSSystemObejcts
 *
 * If ITK_USE_PTHREADS is defined, then
 * pthread_create() will be used to create multiple threads (on
 * a sun, for example).
 */

#ifdef ITK_USE_PTHREADS
#define ITK_MAX_THREADS              128
#endif

#ifdef ITK_USE_WIN32_THREADS
#define ITK_MAX_THREADS              128
#endif

// cygwin threads are unreliable
#ifdef __CYGWIN__
#undef ITK_MAX_THREADS
#define ITK_MAX_THREADS 128
#endif

// at one point, mingw threads caused crashes so limit was set to 1
// as of July 2009, all tests are passing with threads enabled
#if defined( __MINGW32__ )
#undef ITK_MAX_THREADS
#define ITK_MAX_THREADS 128
#endif

#ifndef ITK_MAX_THREADS
#define ITK_MAX_THREADS 1
#endif

/** \par Note
 * If ITK_USE_PTHREADS is defined, then the multithreaded
 * function is of type void *, and returns NULL
 * Otherwise the type is void which is correct for WIN32
 */
#ifdef ITK_USE_PTHREADS
typedef void *( * ThreadFunctionType )(void *);
typedef pthread_t ThreadProcessIDType;
#define ITK_THREAD_RETURN_VALUE  NULL
#define ITK_THREAD_RETURN_TYPE   void *
#endif

#ifdef ITK_USE_WIN32_THREADS
typedef LPTHREAD_START_ROUTINE ThreadFunctionType;
typedef HANDLE                 ThreadProcessIDType;
#define ITK_THREAD_RETURN_VALUE 0
#define ITK_THREAD_RETURN_TYPE DWORD __stdcall
#endif

#ifndef ITK_THREAD_RETURN_VALUE
typedef void ( *ThreadFunctionType )(void *);
typedef int     ThreadProcessIDType;
#define ITK_THREAD_RETURN_VALUE
#define ITK_THREAD_RETURN_TYPE void
#endif

class ITKCommon_EXPORT MultiThreader:public Object
{
public:
  /** Standard class typedefs. */
  typedef MultiThreader              Self;
  typedef Object                     Superclass;
  typedef SmartPointer< Self >       Pointer;
  typedef SmartPointer< const Self > ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(MultiThreader, Object);

  /** Get/Set the number of threads to create. It will be clamped to the range
   * [ 1, m_GlobalMaximumNumberOfThreads ], so the caller of this method should
   * check that the requested number of threads was accepted. */
  void SetNumberOfThreads(int numberOfThreads);

  itkGetConstMacro(NumberOfThreads, int);

  /** Set/Get the maximum number of threads to use when multithreading.  It
   * will be clamped to the range [ 1, ITK_MAX_THREADS ] because several arrays
   * are already statically allocated using the ITK_MAX_THREADS number.
   * Therefore the caller of this method should check that the requested number
   * of threads was accepted. */
  static void SetGlobalMaximumNumberOfThreads(int val);

  static int  GetGlobalMaximumNumberOfThreads();

  /** Set/Get the value which is used to initialize the NumberOfThreads in the
   * constructor.  It will be clamped to the range [1, m_GlobalMaximumNumberOfThreads ].
   * Therefore the caller of this method should check that the requested number
   * of threads was accepted. */
  static void SetGlobalDefaultNumberOfThreads(int val);

  static int  GetGlobalDefaultNumberOfThreads();

  /** Execute the SingleMethod (as define by SetSingleMethod) using
   * m_NumberOfThreads threads. As a side effect the m_NumberOfThreads will be
   * checked against the current m_GlobalMaximumNumberOfThreads and clamped if
   * necessary. */
  void SingleMethodExecute();

  /** Execute the MultipleMethods (as define by calling SetMultipleMethod for
   * each of the required m_NumberOfThreads methods) using m_NumberOfThreads
   * threads. As a side effect the m_NumberOfThreads will be checked against the
   * current m_GlobalMaximumNumberOfThreads and clamped if necessary. */
  void MultipleMethodExecute();

  /** Set the SingleMethod to f() and the UserData field of the
   * ThreadInfoStruct that is passed to it will be data.
   * This method (and all the methods passed to SetMultipleMethod)
   * must be of type itkThreadFunctionType and must take a single argument of
   * type void *. */
  void SetSingleMethod(ThreadFunctionType, void *data);

  /** Set the MultipleMethod at the given index to f() and the UserData
   * field of the ThreadInfoStruct that is passed to it will be data. */
  void SetMultipleMethod(int index, ThreadFunctionType, void *data);

  /** Create a new thread for the given function. Return a thread id
     * which is a number between 0 and ITK_MAX_THREADS - 1. This
   * id should be used to kill the thread at a later time. */
  // FIXME: Doesn't seem to be called anywhere...
  int SpawnThread(ThreadFunctionType, void *data);

  /** Terminate the thread that was created with a SpawnThreadExecute() */
  void TerminateThread(int thread_id);

  /** This is the structure that is passed to the thread that is
   * created from the SingleMethodExecute, MultipleMethodExecute or
   * the SpawnThread method. It is passed in as a void *, and it is up
   * to the method to cast correctly and extract the information.  The
   * ThreadID is a number between 0 and NumberOfThreads-1 that
   * indicates the id of this thread. The NumberOfThreads is
   * this->NumberOfThreads for threads created from
   * SingleMethodExecute or MultipleMethodExecute, and it is 1 for
   * threads created from SpawnThread.  The UserData is the (void
   * *)arg passed into the SetSingleMethod, SetMultipleMethod, or
   * SpawnThread method. */
#ifdef ThreadInfoStruct
#undef ThreadInfoStruct
#endif
  struct ThreadInfoStruct {
    int ThreadID;
    int NumberOfThreads;
    int *ActiveFlag;
    MutexLock::Pointer ActiveFlagLock;
    void *UserData;
    ThreadFunctionType ThreadFunction;
    enum { SUCCESS, ITK_EXCEPTION, ITK_PROCESS_ABORTED_EXCEPTION, STD_EXCEPTION, UNKNOWN } ThreadExitCode;
  };
protected:
  MultiThreader();
  ~MultiThreader();
  void PrintSelf(std::ostream & os, Indent indent) const;

private:
  MultiThreader(const Self &);  //purposely not implemented
  void operator=(const Self &); //purposely not implemented

  /** An array of thread info containing a thread id
   *  (0, 1, 2, .. ITK_MAX_THREADS-1), the thread count, and a pointer
   *  to void so that user data can be passed to each thread. */
  ThreadInfoStruct m_ThreadInfoArray[ITK_MAX_THREADS];

  /** The methods to invoke. */
  ThreadFunctionType m_SingleMethod;
  ThreadFunctionType m_MultipleMethod[ITK_MAX_THREADS];

  /** Storage of MutexFunctions and ints used to control spawned
   *  threads and the spawned thread ids. */
  int                 m_SpawnedThreadActiveFlag[ITK_MAX_THREADS];
  MutexLock::Pointer  m_SpawnedThreadActiveFlagLock[ITK_MAX_THREADS];
  ThreadProcessIDType m_SpawnedThreadProcessID[ITK_MAX_THREADS];
  ThreadInfoStruct    m_SpawnedThreadInfoArray[ITK_MAX_THREADS];

  /** Internal storage of the data. */
  void *m_SingleData;
  void *m_MultipleData[ITK_MAX_THREADS];

  /** Global variable defining the maximum number of threads that can be used.
   *  The m_GlobalMaximumNumberOfThreads must always be less than or equal to
   *  ITK_MAX_THREADS and greater than zero. */
  static int m_GlobalMaximumNumberOfThreads;

  /*  Global variable defining the default number of threads to set at
   *  construction time of a MultiThreader instance.  The
   *  m_GlobalDefaultNumberOfThreads must always be less than or equal to the
   *  m_GlobalMaximumNumberOfThreads and larger or equal to 1 once it has been
   *  initialized in the constructor of the first MultiThreader instantiation.
   */
  static int m_GlobalDefaultNumberOfThreads;

  /** The number of threads to use.
   *  The m_NumberOfThreads must always be less than or equal to
   *  the m_GlobalMaximumNumberOfThreads before it is used during the execution
   *  of a threaded method. Its value is clamped in the SingleMethodExecute()
   *  and MultipleMethodExecute(). Its value is initialized to
   *  m_GlobalDefaultNumberOfThreads at construction time. Its value is clamped
   *  to the current m_GlobalMaximumNumberOfThreads in the
   *  SingleMethodExecute() and MultipleMethodExecute() methods.
   */
  int m_NumberOfThreads;

  /** Static function used as a "proxy callback" by the MultiThreader.  The
   * threading library will call this routine for each thread, which
   * will delegate the control to the prescribed SingleMethod. This
   * routine acts as an intermediary between the MultiThreader and the
   * user supplied callback (SingleMethod) in order to catch any
   * exceptions thrown by the threads. */
  static ITK_THREAD_RETURN_TYPE SingleMethodProxy(void *arg);

  /** Spawn a thread for the prescribed SingleMethod.  This routine
   * spawns a thread to the SingleMethodProxy which runs the
   * prescribed SingleMethod.  The SingleMethodProxy allows for
   * exceptions within a thread to be naively handled. A similar
   * abstraction needs to be added for MultipleMethod and
   * SpawnThread. */
  ThreadProcessIDType DispatchSingleMethodThread(ThreadInfoStruct *);

  /** Wait for a thread running the prescribed SingleMethod. A similar
   * abstraction needs to be added for MultipleMethod (SpawnThread
   * already has a routine to do this. */
  void WaitForSingleMethodThread(ThreadProcessIDType);

  /** Friends of Multithreader.
   * ProcessObject is a friend so that it can call PrintSelf() on its
   * Multithreader. */
  friend class ProcessObject;
};
}  // end namespace itk
#endif
