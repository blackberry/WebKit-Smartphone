/*
 * Copyright (C) 2009 Research in Motion Ltd. http://www.rim.com/
 */

#ifndef jsc_h
#define jsc_h

struct JSCPromptCallback
{
    virtual void setResult(const unsigned short* result, unsigned length) = 0;
};

struct JSCClient
{
    virtual void output(const char*) = 0;
    virtual void alert(const unsigned short*, unsigned length) = 0;
    virtual bool confirm(const unsigned short*, unsigned length) = 0;
    virtual void prompt(const unsigned short* message, unsigned messageLength
        , const unsigned short* defaultValue, unsigned defaultValueLength, JSCPromptCallback* setResult) = 0;
};

void jscInitialize();
void jscCleanup();
void jscSetClient(JSCClient* client);
int jscRunScript(const char* buffer);

#endif // jsc_h