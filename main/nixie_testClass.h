/*
    ...

*/
#ifndef __NIXIE_TESTCLASS_H__
#define __NIXIE_TESTCLASS_H__


// 
typedef struct{
    int iYetAnotherInt;
    bool bfirstEverBool;
} myTestClass_arg_t;




// nixie-specific webserver class
class MyTestClass
{
private:
    const char *TAG = "nixie_test_class";
    
    static int iAStaticLocalVariable;


public:
    MyTestClass(/* args */);
    ~MyTestClass();

    // lets try and create some local arguments
    myTestClass_arg_t arg;

    static void freeRtosTask(void *arg);


};




#endif /* __NIXIE_TESTCLASS_H__ */