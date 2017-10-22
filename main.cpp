#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <assert.h>

#define assert(e)       ((e) ? (void)0 : _assert(#e, __FILE__, __LINE__))

#define init_alloc(type, number)     alloc<type, true> (number, #type, __LINE__, __FILE__)
#define noinit_alloc(type, number)   alloc<type, false>(number, #type, __LINE__, __FILE__)

template<class T>
void __alloc_init__(T* begin, T* end){
    while(begin != end){
        new (begin) T();
        ++begin;
    }
}

template<class T, bool init = false>
T* alloc(std::size_t number, const char* type_name, int line, const char* file){
    if (number <= 0) return nullptr;
    T* retval = (T*) malloc(number * sizeof(T));
    if (retval == nullptr) {
        fprintf(stderr, "-----------------------------------\n"
            "ERROR: Memory allocation failed.\n"
            "Function: alloc<%s, %s>(%u)\n"
            "Called by: %s(%s, %u)\n"
            "Required memory: %u * %u bytes\n"
            "Call point: file %s, line %d \n"
            "-----------------------------------\n",
            type_name, ((init)? "true": "false"), number,
            ((init)? "init_alloc": "noinit_alloc"), type_name, number,
            number, sizeof(T),
            file, line
        );
        return nullptr;
    }
    if (init)
        __alloc_init__(retval, retval + number);
    return retval;
}

//not used
int binpow(int a, long long n, int modulus) {
	long long res = 1;
	while (n)
		if (n & 1) {
			res *= a;
			res %= modulus;
			--n;
		}
		else {
			a *= a;
			a %= modulus;
			n >>= 1;
		}
    res %= modulus;
	return res;
}

struct StackErr{
    enum : unsigned char{
        stack_ok = 0,
        bad_alloc = 1,
        stack_cut = 2,
        hash_error = 3,
        canary = 4
    };
    static char* error_info(int error);
};

char* StackErr::error_info(int error){
    char* s = (char*) calloc(100, 1);
    switch(error){
        case StackErr::stack_ok: {sprintf(s, "CStack ok"); break;}
        case StackErr::bad_alloc: {sprintf(s, "Call of CStack::resize or CStack::CStack caused bad_alloc error"); break;}
        case StackErr::stack_cut: {sprintf(s, "Call of CStack::resize cut off CStack"); break;}
        case StackErr::hash_error: {sprintf(s, "Call of CStack::is_ok caused error of hash, which means that someone screw up your stack..."); break;}
        case StackErr::canary: {sprintf(s, "Call of CStack::is_ok caused canary protection delete"); break;}
    }
    return s;
}

//-----
template<class T>
void print(const T& value, FILE* f);
//-----

//-----
template<class T>
int hash(const T& value);
//-----

#define control

template<class T>
class CStack{
    int canaryleft;

    T* begin;
    T* end;
    T* curr;

    long long data_hash;
    long long stack_hash;

    const int base = 5;
    const int modulus = 1000000007;
    const int basic_value = 0xfdca;
    long long get_data_hash() const;
    long long get_stack_hash() const;

    int canaryright;
    void size_ctor(std::size_t start_size);
public:
    mutable int error;
    CStack() : canaryleft(0xfd0b), begin(nullptr), end(nullptr), curr(nullptr), canaryright(0xfd0b), error(0) {}
    CStack(std::size_t start_size) {size_ctor(start_size);}

    void resize(std::size_t new_size);

    void push(const T& element);
    void pop ();
    void pop (T& element) {pop(); element = *curr;}
    const T& top() const;

    std::size_t size() const {return curr - begin;}
    std::size_t capacity() const {return end - begin;}

    void dump(FILE* f, const T& defult_value = T()) const;
    void is_ok() const ;
};

template<class T>
void CStack<T>::size_ctor(std::size_t start_size){
#define POISON 0xfd0b
    begin = init_alloc(T, start_size + 2);
    if (begin == nullptr) {error = StackErr::bad_alloc; return;}

    ++begin;
    *(begin - 1) = POISON;
    *(begin + start_size) = POISON;

    end   = begin + start_size;
    curr  = begin;
    error = 0;
    stack_hash = get_stack_hash();
    data_hash = get_data_hash();
#undef POISON
}

template<class T>
void CStack<T>::resize(std::size_t new_size){
#define POISON 0xfd0b
    if (begin == nullptr){
        size_ctor(new_size);
        return;
    }

    #ifdef control
        is_ok();
    #endif // control

    if (new_size == capacity()) return;
    if (new_size <  size()) {error = StackErr::stack_cut;}

    T* new_begin = init_alloc(T, new_size + 2);
    if (new_begin == nullptr) {error = StackErr::bad_alloc; return;}

    ++new_begin;
    *(new_begin - 1) = POISON;
    *(new_begin + new_size) = POISON;

    curr = begin + ((new_size < size())? new_size : size());
    std::copy(begin, curr, new_begin);

    curr = new_begin + size();
    delete[] (begin - 1);
    begin = new_begin;
    end = begin + new_size;
    stack_hash = get_stack_hash();
    data_hash = get_data_hash();

    #ifdef control
        is_ok();
    #endif // control
#undef POISON
}

template<class T>
void CStack<T>::push(const T& element){
    if (begin == nullptr) resize(1);

    #ifdef control
        is_ok();
    #endif // control

    if ((error)&&(error != StackErr::stack_cut)) return;
    if (size() == capacity()) resize((end - begin) * 2);

    *curr = element;
    ++curr;

    data_hash = get_data_hash();
    stack_hash = get_stack_hash();

    #ifdef control
        is_ok();
    #endif // control
}

template<class T>
void CStack<T>::pop(){
    #ifdef control
        is_ok();
    #endif // control

    assert(curr != begin);
    if ((error)&&(error != StackErr::stack_cut)) return;
    if (size() * 4 <= capacity()) resize(size() * 2);
    --curr;
    //data_hash -= (((binpow(base, curr - begin, modulus) * hash(*curr)) % modulus) + modulus) % modulus;
    data_hash = get_data_hash();
    stack_hash = get_stack_hash();

    #ifdef control
        is_ok();
    #endif // control
}

template<class T>
const T& CStack<T>::top() const{
    #ifdef control
        is_ok();
    #endif // control

    assert(curr != begin);
    if ((error)&&(error != StackErr::stack_cut)) return T();
    return *(curr - 1);
}

template<class T>
void CStack<T>::dump(FILE* f, const T& default_value) const{
#define POISON 0xfd0b
    fprintf(f, "-----------------------------------\n");
    if ((error == StackErr::hash_error)||(error == StackErr::canary))
        fprintf(f, "Error: is_ok contol not passed.\n");
    fprintf(f, "Dump of CStack at adress [%p]\n", this);
    fprintf(f, "\tbegin = %p\n\tend = %p\n\tcurr = %p\n", begin,  end, curr);
    fprintf(f, "\tcanaries:\n"
               "\t\tleft canary save = %s\n"
               "\t\tleft canary save = %s\n"
               "\t\t\"data[-1]\" save = %s\n"
               "\t\t\"data[%d]\" save = %s\n",
                (canaryleft  == POISON)? "true": "false",
                (canaryright == POISON)? "true": "false",
                (*(begin - 1) == POISON)? "true": "false",
                capacity(), (*end == POISON)? "true": "false"
    );
    fprintf(f, "\tdata:\n");
    T* it = begin;
    T* same = nullptr;
    while(it != end){
        fprintf(f, "\t\tdata[%u] = ", (it - begin));
        print(*it, f);
        if (*it == default_value) fprintf(f, "; default value\n");
        else fprintf(f, ";\n");
        same = it;
        while(*it == *same) ++it;
        if (it - same > 1) {fprintf(f, "\t\t...\n"); --it;}
    }
    fprintf(f, "-----------------------------------\n");
#undef POISON
}

template<class T>
long long CStack<T>::get_data_hash() const {
    long long retval = basic_value;
    long long factor = 1;
    for(T* it = begin; it != curr; ++it){
        retval += hash(*it) * factor;
        retval %= modulus;
        factor *= base;
        factor %= modulus;
    }
    //printf("%ll", retval);
    return retval;
}

template<class T>
long long CStack<T>::get_stack_hash() const {
    long long retval = basic_value;
    retval +=   (unsigned long long) begin % modulus;
    retval += (((unsigned long long) end % modulus)) * base % modulus;
    retval += (((unsigned long long) curr % modulus) * base % modulus) * base % modulus;
    return retval;
}

template<class T>
void CStack<T>::is_ok() const{
#define POISON 0xfd0b
    if ((canaryleft != POISON)||(canaryright != POISON)) error = StackErr::canary;
    if ((*(begin - 1) != POISON)||(*(end) != POISON)) error = StackErr::canary;
    if (get_stack_hash() != stack_hash) error = StackErr::hash_error;
    if (get_data_hash()  != data_hash)  error = StackErr::hash_error;
    if ((error == StackErr::hash_error)||(error == StackErr::canary)) dump(stderr);
#undef POISON
}

template<>
void print<int>(const int& value, FILE* f){
    fprintf(f, "%d", value);
}
template<>
int hash<int>(const int& value){
    return value;
}

int main()
{
    CStack<int> a;
    for(int i = 0; i < 200; ++i)
        a.push(i * 15 + 5);

    a.dump(stdout);
    return 0;
}
