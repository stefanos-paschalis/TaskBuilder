#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <chrono>
#include <map>
#include <iomanip>
#include <assert.h>

using namespace std;
using namespace chrono;
using namespace std::chrono_literals;

using Clock = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::time_point<Clock>;
using Seconds = std::chrono::duration<long double>;

class Tools {
public:
    static string time_point_to_string(TimePoint tp) {
        time_t tt = std::chrono::system_clock::to_time_t(tp);
        string s(20, '\0');
        strftime(&s[0], s.size(), "%d/%m/%Y %H:%M:%S", localtime(&tt));
        return s;
    }

    static TimePoint string_to_time_point(const string& date) {
        int dd, mm, yyyy, hh24, mi, ss;
        sscanf(date.c_str(), "%d/%d/%d %d:%d:%d", &dd, &mm, &yyyy, &hh24, &mi, &ss);
        tm tm{0};
        tm.tm_year = yyyy - 1900; // Year since 1900
        tm.tm_mon = mm - 1;     // 0-11
        tm.tm_mday = dd;        // 1-31
        tm.tm_hour = hh24;        // 0-23
        tm.tm_min = mi;         // 0-59
        tm.tm_sec = (int)ss;    // 0-61 (0-60 in C++11)
        time_t tt = mktime(&tm);
        return std::chrono::system_clock::from_time_t(tt);
    }
};

class TaskBuilder;
class TaskWithBuilder;
class TaskDoingBuilder;
class TaskRunsBuilder;
class TaskBeginsBuilder;

class Task {
    string name{""};
    string action{""};
    Seconds period{0};
    TimePoint last_run{Clock::from_time_t(0)};
    TimePoint next_run{Clock::from_time_t(0)};
    Task() = default;

public:
    friend class TaskBuilder;
    friend class TaskWithBuilder;
    friend class TaskDoingBuilder;
    friend class TaskRunsBuilder;
    friend class TaskBeginsBuilder;

    static TaskBuilder create();

    friend ostream &operator<<(ostream &os, const Task &task) {
        os << "name: " << task.name
           << ", action: " << task.action
           << ", period: " << task.period.count() << " seconds"
           << ", last_run: " << Tools::time_point_to_string(task.last_run)
           << ", next_run: " << Tools:: time_point_to_string(task.next_run);
        return os;
    }
};

class TaskBuilderBase
{
protected:
    Task& task;
    explicit TaskBuilderBase(Task& task) : task{ task } {}
public:
    operator Task() const { return std::move(task); }
    TaskWithBuilder with() const;
    TaskDoingBuilder doing() const;
    TaskRunsBuilder runs() const;
    TaskBeginsBuilder begins() const;
};

class TaskBuilder : public TaskBuilderBase {
    Task t;
public:
    TaskBuilder() : TaskBuilderBase{t} {}
};

TaskBuilder Task::create()
{
    return TaskBuilder{};
}

class TaskWithBuilder : public TaskBuilderBase {
    typedef TaskWithBuilder self;
public:
    explicit TaskWithBuilder(Task &task) : TaskBuilderBase{task} {}

    self& name(const string& name) {
        task.name = name;
        return *this;
    }
};

class TaskDoingBuilder : public TaskBuilderBase {
    typedef TaskDoingBuilder self;
public:
    explicit TaskDoingBuilder(Task &task) : TaskBuilderBase{task} {}

    self& action(const string& action) {
        task.action = action;
        return *this;
    }
};

class TaskRunsBuilder : public TaskBuilderBase {
    typedef TaskRunsBuilder self;
public:
    explicit TaskRunsBuilder(Task& task) : TaskBuilderBase{task} {}

    self& once() {
        task.period = (Seconds)0;
        return *this;
    }

    self& every(long double period) {
        task.period = (Seconds)period;
        return *this;
    }

    self& second() { task.period = (Seconds)1.0; return *this; }
    self& seconds() { task.period *= 1.0; return *this; }
    self& minute() { task.period = (Seconds)60.0; return *this; }
    self& minutes() { task.period *= 60.0; return *this; }
    self& hour() { task.period = (Seconds)(60.0*60.0); return *this; }
    self& hours() { task.period *= (60.0*60.0); return *this; }
    self& day() { task.period = (Seconds)(60.0*60.0*24.0);  return *this; }
    self& days() { task.period *= (60.0*60.0*24.0);  return *this; }
    self& week() { task.period = (Seconds)(60.0*60.0*24.0*7.0);  return *this; }
    self& weeks() { task.period *= (60.0*60.0*24.0*7.0);  return *this; }
    self& month() { task.period = (Seconds)(60.0*60.0*24.0*7.0*30.0);  return *this; }
    self& months() { task.period *= (60.0*60.0*24.0*7.0*30.0);  return *this; }
    self& year() { task.period = (Seconds)(60.0*60.0*24.0*7.0*30.0*365.0);  return *this; }
    self& years() { task.period *= (60.0*60.0*24.0*7.0*30.0*365.0);  return *this; }
};

class TaskBeginsBuilder : public TaskBuilderBase {
    typedef TaskBeginsBuilder self;
public:
    explicit TaskBeginsBuilder(Task& task) :TaskBuilderBase{task} {}

    self& on(const string& date) {
        task.next_run = Tools::string_to_time_point(date);
        return *this;
    }
};

TaskWithBuilder TaskBuilderBase::with() const {
    return TaskWithBuilder{task};
}

TaskDoingBuilder TaskBuilderBase::doing() const {
    return TaskDoingBuilder{task};
}

TaskRunsBuilder TaskBuilderBase::runs() const {
    return TaskRunsBuilder{task};
}

TaskBeginsBuilder TaskBuilderBase::begins() const {
    return TaskBeginsBuilder{task};
}

int main() {
    Task t1 = Task::create()
            .with().name("Task1")
            .doing().action("Run antivirus")
            .runs().every(1).week()
            .begins().on("12/01/2020 06:00:01");

    cout << t1 << endl;

    Task t2 = Task::create().runs().once().begins().on("12/01/2019 06:00:01");

    cout << t2 << endl;
    
    return 0;
}
