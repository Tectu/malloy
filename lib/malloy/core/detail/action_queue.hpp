#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>

#include <queue>

namespace malloy::detail
{

    /**
     * @class action_queue
     * @brief Stores and executes functions
     * @details The next queued function is not executed until the callback passed to the last executed function is called
     * @note All methods in this class are threadsafe
     * @note This class will be upgraded to coroutines once they become sufficently supported, see https://github.com/Tectu/malloy/issues/70 for more details
     * @warning The callback passed must be called for the queue to continue running
     * @warning Objects of this class do no explicit lifetime management for themselves. It is up to the owner of the object to ensure it is still alive when the callback is invoked
     * @tparam Executor The executor to use
     */
    template<typename Executor>
    class action_queue
    {
        template<typename... Args>
        class basic_action
        {
        public:
            virtual void invoke(Args...) = 0;

            void operator()(Args... args)
            {
                invoke(std::forward<Args>(args)...);
            }

            virtual ~basic_action() = default;
        };
    public:
        using act_args = basic_action<>&;
        using action = basic_action<act_args>;

    private:
        using act_t = std::unique_ptr<action>;    //std::function<void(const std::function<void()>&)>;
        using acts_t = std::queue<act_t>;
        using ioc_t = boost::asio::strand<Executor>;

        class on_done_delegate : public basic_action<> {
        public:
            action_queue* parent;

            explicit on_done_delegate(action_queue* parent) : parent{parent} {}

            void invoke() override
            {
                if (!parent->m_acts.empty()) {
                    parent->exe_next();
                } else {
                    parent->m_currently_running_act = false;
                }
            }
        };

    public:
        /**
         * @brief Construct the action queue. It will not execute anything until run() is called.
         *
         * @param ioc The strand to use for synchronisation
         */
        explicit action_queue(ioc_t ioc) :
            m_ioc{std::move(ioc)}
        {
        }

        /**
         * @brief Add an action to the queue.
         *
         * @note If run() has been called and the queue is currently empty, the new action will run immediately.
         * @note This uses a custom class rather than std::function to enable non-copyable delegates being used
         *
         * @param act
         */
        template<std::derived_from<action> Act>
        void push(Act&& act) requires(std::is_move_constructible_v<Act>)
        {
            boost::asio::dispatch(m_ioc, [this, act = std::move(act)]() mutable -> void {
                m_acts.push(std::make_unique<Act>(std::move(act)));
                if (!m_currently_running_act) {
                    run();
                }
            });
        }

        /**
         * @brief Starts the queue running, if it isn't already.
         *
         * @note Only needs to be called once.
         */
        void run()
        {
            m_running = true;
            exe_next();
        }

    private:
        void exe_next()
        {
            // Running now...
            m_currently_running_act = true;

            // Execute
            boost::asio::dispatch(m_ioc, [this] {
                if (!m_acts.empty()) {
                    auto act = std::move(m_acts.front());
                    m_acts.pop();
                    act->invoke(m_on_done);
                }
                // Nothing left to do...
                else {
                    m_currently_running_act = false;
                }
            });
        }

        acts_t m_acts;
        ioc_t m_ioc;
        std::atomic_bool m_running{false};
        std::atomic_bool m_currently_running_act{false};
        on_done_delegate m_on_done{this};
    };

    }    // namespace malloy::detail
