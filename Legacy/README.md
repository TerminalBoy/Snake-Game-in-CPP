This is an old implementation of how I used raw pointers of snake_part to make my dynamic resizable array.
And used a snake_part::transform(int f_max_size) to adjust the size of snake_part*

This method was depricated in latest commits for memory safety reasons

Currently std::vector<typename T> is used for dynamic arrays of snake_part
