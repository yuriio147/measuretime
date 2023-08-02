# measuretime
A small header only util to measure execution time

## Examples
```c++
#include <iostream>
#include <random>

#include "measuretime.h"

void random_func()
{
    debugScopeTime(__FUNCTION__);

    std::mt19937_64 rng;
    uint64_t timeSeed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::seed_seq ss{uint32_t(timeSeed & 0xffffffff), uint32_t(timeSeed>>32)};
    rng.seed(ss);
    std::uniform_real_distribution<double> unif(0, 1);
    for (int i = 0; i < 10; i++)
    {
        unif(rng);
    }
}

int main()
{
    random_func();
    random_func();
    random_func();
    return 0;
}
```
Output:
```
random_func 15 [us] avg. 15 [us] cnt. 1 ttl. 15 [us]
random_func 8 [us] avg. 11.5 [us] cnt. 2 ttl. 23 [us]
random_func 7 [us] avg. 10 [us] cnt. 3 ttl. 30 [us]
```

```c++
#include <iostream>
#include <random>

#include "measuretime.h"

void random_func()
{
    std::mt19937_64 rng;
    uint64_t timeSeed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::seed_seq ss{uint32_t(timeSeed & 0xffffffff), uint32_t(timeSeed>>32)};
    rng.seed(ss);
    std::uniform_real_distribution<double> unif(0, 1);
    for (int i = 0; i < 1e6; i++)
    {
        unif(rng);
    }
    debugTimeLog(std::string{"label"});
}

int main()
{
    debugTime(std::string{"label"});

    random_func();
    random_func();
    random_func();
    random_func();
    random_func();

    debugTimeEnd(std::string{"label"});
    return 0;
}
```
Output:
```
'label' log: 58 [us]
'label' log: 94 [us]
'label' log: 103 [us]
'label' log: 112 [us]
'label' log: 121 [us]
'label' end: 122 [us]
```
