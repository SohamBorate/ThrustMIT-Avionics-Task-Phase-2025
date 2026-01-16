#ifndef MOVING_AVG_H
#define MOVING_AVG_H

#include <stdint.h>

template <typename T, int N>
class Moving_Average {
    public:
        int end;
        T curr_sum;
        T curr_avg;
        T values[N];

        Moving_Average() {
            end = N - 1;
            curr_sum = (T)0;
            curr_avg = (T)0;
            for (int i = 0; i < N; i++) {
                values[i] = 0;
            }
        };

        T append(T val) {
            T subtract_val = values[end];
            values[end] = val;
            curr_sum += val - subtract_val;
            curr_avg = curr_sum / (T)N;
            end--;
            if (end == -1) {
                end = N - 1;
            }

            return curr_avg;
        };
};

#endif
