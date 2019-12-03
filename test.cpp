#include <stdio.h>

class Foo{
    public:
        Foo(int bar): foobar(bar)
        {}

        int *get_foobar_addr(){
            return &foobar;
        }

        int get_foobar(){
            return foobar;
        }

        int foobar;
};

int main(){
    Foo * x = new Foo(100);
    int Foo::*foobar = &x->get_foobar();
    printf("%p", x->get_foobar_addr());
    printf("%p", &x->get_foobar());
}