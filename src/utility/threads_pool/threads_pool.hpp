/* FILE NAME   : 'threads_pool.hpp'
 * PURPOSE     : Threads pool control class implementation file.
 * PROGRAMMER  : Fedor Borodulin.
 * LAST UPDATE : 07.04.2023.
 * NOTE        : Module namespace 'prj'.
 */

#ifndef __threads_pool_hpp__
#define __threads_pool_hpp__

#include <def.h>

/* Project namespace // Utility module */
namespace prj::util
{
  /* Auxilary class for easier distributing approximatly equal multi-threaded tasks */
  template<typename task_data, size_t task_batch = 1>
    class threads_pool
    {
      static_assert(task_batch > 0, "Cannot use batch size less or equal zero!");

    private:
      /* Storage of tasks data */
      std::list<task_data> TaskPool {};

      /* Thread storage structure */
      struct thread
      {
        std::thread ThreadHandle;
        std::list<task_data *> Tasks;
        bool RunFlag = true;
      }; /* end of 'thread' structure */

      /* Threads pool */
      std::list<thread> ThreadsPool {};

      /* Pause flag */
      bool PauseFlag = true;

      /* Task function */
      std::function<bool( task_data * )> ThreadFunction {};

    public:
      /* Default constructor */
      threads_pool( void ) {}

      /* Constructor from task function */
      threads_pool( const std::function<bool( task_data * )> &ThreadFunc ) :
        ThreadFunction {ThreadFunc}
      { }

      /* Destructor */
      ~threads_pool( void )
      {
        Terminate();
      } /* End of destructor */

      /* Thread function setting function
       * ! Terminates all tasks
       */
      void SetFunction( const std::function<bool( task_data * )> &ThreadFunc )
      {
        Terminate();

        ThreadFunction = ThreadFunc;
      } /* End of 'SetFunction' function */

      /* Task running function
       * ARGUMENTS:
       *    - Avalible threads count (default: 0 <=> auto):
       *        size_t Threads;
       */
      void Run( size_t Threads = 0 )
      {
        /* Check there is function */
        if (!ThreadFunction)
          return;

        /* Pause all previous */
        Pause();

        /* Auto evaluation */
        if (Threads == 0)
          Threads = std::max<long long>((long long)(std::thread::hardware_concurrency() >> 1),
                                        (long long)(std::thread::hardware_concurrency() - 2));
        else
          Threads = std::clamp<size_t>(Threads, 1, std::thread::hardware_concurrency() << 1);

        /* Auxilary checks */
        if (TaskPool.empty())
          return;

        Threads = std::min(Threads, TaskPool.size());

        /* Run threads */
        {
          const size_t
            TasksPerThread {TaskPool.size() / Threads},
            TasksPerThreadOverhead {TaskPool.size() % Threads};

          auto TaskIt {TaskPool.begin()};

          /* Thread function temporary copy */
          const auto ThreadFunction {this->ThreadFunction};

          for (size_t i {0}, made_tasks {0}; i < Threads; i++)
          {
            const size_t Tasks {TasksPerThread + (size_t)(i < TasksPerThreadOverhead)};

            auto *ThreadData {&ThreadsPool.emplace_back()};

            for (size_t j {0}; j < Tasks; j++, ++TaskIt)
              ThreadData->Tasks.push_back(&TaskIt._Ptr->_Myval);

            ThreadData->ThreadHandle = std::move(std::thread {[ThreadData, ThreadFunction]( void ) -> int
            {
              /* Tasks mainloop */
              while (ThreadData->RunFlag &&
                     !ThreadData->Tasks.empty())
              {
                for (auto Task {ThreadData->Tasks.begin()};
                     Task != ThreadData->Tasks.end();)
                {
                  size_t Cnt {task_batch - 1};
                  bool EndTask {false};

                  do
                  {
                    bool FinishedTask = ThreadFunction(*Task);

                    if (FinishedTask)
                    {
                      EndTask = true;
                      break;
                    }
                  } while (--Cnt);

                  if (EndTask)
                  {
                    auto Old {Task};

                    ++Task;
                    ThreadData->Tasks.erase(Old);
                  }
                  else
                    ++Task;
                }

                Sleep(0);
              }

              return 0;
            }});
          }
        }

        PauseFlag = false;
      } /* End of 'Run' function */

      /* Task pause funciton */
      void Pause( void )
      {
        if (!PauseFlag)
        {
          PauseFlag = true;

          for (auto &Elm : ThreadsPool)
            Elm.RunFlag = false;

          while (!ThreadsPool.empty())
          {
            auto &Elm {ThreadsPool.front()};

            Elm.ThreadHandle.join();

            ThreadsPool.pop_front();
          }
        }
      } /* End of 'Pause' function */

      /* Task termination funciton */
      void Terminate( void )
      {
        /* Pause all */
        Pause();

        /* Delete tasks */
        TaskPool.clear();
      } /* End of 'Terminate' function */

      /* Task adding function */
      template<typename ...args>
        void AddTask( args &&...Args )
        {
          TaskPool.emplace_back(std::forward<args>(Args)...);
        } /* End of 'AddTask' function */
    }; /* end of 'threads_pool' class */
} /* end of 'prj::util' namespace */

#endif /* __threads_pool_hpp__ */

/* END OF 'threads_pool.hpp' FILE */
