/*
 * @Author: snow pang
 * @Date: 2019-11-04 14:26:07
 * @LastEditors: snow pang
 * @LastEditTime: 2019-11-16 23:05:42
 * @FilePath: \C++Embedded\main.cpp
 * @Description: 
 */

#include <iostream>
#include <string>
#include <list>
#include <map>
using namespace std;


class Transition;
class EventTransition;
class Fsm;
unsigned int times_count = 0;
unsigned int get_timestamp()
{
    return times_count;
}
//抽象状态类
//包含两部分 状态和动作
class State
{
    public:
        State(int state_id):
        state_id(state_id){}
        virtual void enter() = 0;
        virtual void update() = 0;
        virtual void exit() = 0;
        virtual bool check() = 0; 
        inline int get_to_state_id() const
        {
            return state_to_id;
        }
        
        inline void set_state_to_id(const int to_id)
        {
            state_to_id = to_id;
        }
        
        inline int get_state_id() const
        {
            return state_id;
        }
        
    private:
        int state_id;
        static int state_to_id;
};

int State::state_to_id = -1;

//注册状态表


class Fsm
{
    public:
        Fsm(State *state_init):
        state_cur(state_init),
        initialize(false)
        {
            state_map.clear();
        }

        void add_state(State *state)
        {
            map<int, State*>::iterator it = state_map.find(state->get_state_id());
            if(it != state_map.end())
            {
                cout<<"状态已存在，请勿重复添加"<<endl;
            }else{
                state_map.insert(make_pair(state->get_state_id(), state));
            }
        }
        
        void run()
        {
            if(!initialize)
            {
                initialize = true;
                state_cur->enter();
            }else{
                state_cur->update();
                if(state_cur->check())
                {
                    state_cur->exit();
                    int to_state_id = state_cur->get_to_state_id();
                    state_cur = state_map[to_state_id];
                    state_cur->enter();
                }
            }
        }

       
    private:
        map<int, State*> state_map;
        State* state_cur;
        bool initialize;
};

class TimedTransition
{
    public:
        TimedTransition(unsigned int interval):
        interval(interval),
        start(0){}
          
        bool check_timed(const unsigned int timestamp)
        {
            if(start == 0)
            {
                start = timestamp;
            }else{
                if((timestamp - start) > interval)
                {
                    start = 0;
                    return true;
                }
            }
            return false;
        }
        private:
            unsigned int interval;  //ms
            unsigned int start;  
};

class EventTransition
{
    public:
        EventTransition(int id):event_id(id){}
        
        inline int get_event_id() const 
        {
            return event_id;
        }

    private:
        int event_id;
};


class State_idle:public State
{
    public:
        State_idle():
        State(0x01),
        timed_t_1(1000000),
        count(0)
        {
            timed_t = new TimedTransition(timed_t_1);
        }
        
        virtual void enter()
        {
            count = 0;
            cout<<"idle state enter" << endl;
        }

        virtual void update()
        {
            count++;
        }

        virtual void exit()
        {
            count=0;
            cout << "idle state exit"<< endl;
        }

        virtual bool check()
        {
            unsigned int timestamp = get_timestamp();
            if(count > 100000000000)
            {
                set_state_to_id(0x02);
                cout<< "计数完成" << endl;
                return true;
            }

            if(timed_t->check_timed(timestamp))
            {
                cout<<"超时强制退出"<<endl;
                set_state_to_id(0x03);
                return true;
            }
            return false;
        } 

    private:
        TimedTransition* timed_t;
        unsigned int timed_t_1;
        unsigned int count;
};

class State_test:public State
{
    public:
        State_test():State(0x02),
        count(0){}

        virtual void enter()
        {
            count = 0;
            cout<<"test state enter"<< endl;
            cout<< "count = "<< count <<endl;
        }

        virtual void update()
        {
            count++;
        }

        virtual void exit()
        {
            count = 0;
            cout << "test state exit" <<endl;
            cout<< "count = "<< count <<endl;
        }

        bool check()
        {
            if(count > 1000000000)
            {
                set_state_to_id(0x01);
                return true;
            }else{
                return false;
            }
        }

    private:
        int count;
};

class State_test1:public State
{
    public:
        State_test1():State(0x03),
        count(0){}

        virtual void enter()
        {
            count = 0;
            cout<<"test1 state enter"<< endl;
            cout<< "count = "<< count <<endl;
        }

        virtual void update()
        {
            count++;
        }

        virtual void exit()
        {
            count = 0;
            cout << "test1 state exit" <<endl;
            cout<< "count = "<< count <<endl;
        }

        bool check()
        {
            if(count > 1000000000)
            {
                set_state_to_id(0x02);
                return true;
            }else{
                return false;
            }
        }

    private:
        int count;
};



int main()
{
    times_count++;
    State *state_idle = new State_idle();
    State *state_test = new State_test();
    State *state_test1 = new State_test1();
    Fsm *task_fsm = new Fsm(state_idle);
    task_fsm->add_state(state_idle);
    task_fsm->add_state(state_test);
    task_fsm->add_state(state_test1);

    while(1)
    {
        task_fsm->run();
        times_count++;
    }

    return 0;
}
