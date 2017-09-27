#include <iostream>

#ifndef __SIZE_TYPE__
typedef int size_t;
#endif

template <class T>
class Static_UnSafe_Stack{
    T* data;
    size_t index;
    size_t capacity;
public:
    Static_UnSafe_Stack() = delete;
    explicit Static_UnSafe_Stack(size_t size);
    ~Static_UnSafe_Stack() {delete[] data;}
    inline size_t size() {return capacity;}
    int push(const T& element);
    int pop();
    T& top();
    bool empty() {return index = 0;}
};

template <class T>
Static_UnSafe_Stack<T>::Static_UnSafe_Stack(size_t size){
    capacity = size;
    data = new T[capacity];
    index = 0;
}

template <class T>
int Static_UnSafe_Stack<T>::push(const T& element){
    if (index == capacity) return 1;
    ++index;
    data[index] = element;
    return 0;
}

template <class T>
int Static_UnSafe_Stack<T>::pop() {
    if (index == -1) return 1;
    data[index] = data[0];
    --index;
    return 0;
}

template <class T>
T& Static_UnSafe_Stack<T>::top(){
    return data[index];
}

int main(){
    Static_UnSafe_Stack<int> a(5);
    return 0;
}