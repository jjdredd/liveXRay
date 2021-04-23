#pragma once

#if !defined(_CPPUNWIND)
#error Please enable exceptions...
#endif

#include <exception>

#include "xrCommon/xr_list.h"
#include "xrstring.h"
#include "_std_extensions.h"

template<typename T> class XRCORE_API CLOption;

template<typename T>
class XRCORE_API CLOption
{
public:
	CLOption(const char *flag_name, const T defval, bool req);
	CLOption(const char *flag_name, bool req);
    ~CLOption();
	bool IsProvided() const;          // was the option provided in the CLI?
	T OptionValue() const;            // value provided with the option

    static void CheckArguments();
	friend void ParseCommandLine(const int argc, const char *argv[]);

private:
	xr_string option_name;
	bool required;
	bool provided = false;

	T argument;

    static typename xr_list<CLOption<T> *>::iterator find_option(const char *flag_name);
    static bool parse_option(const char *option, const char *arg);
	static xr_list<CLOption<T> *> options;
};


template<> xr_list<CLOption<bool> *> CLOption<bool>::options = {};
template<> xr_list<CLOption<int> *> CLOption<int>::options = {};
template<> xr_list<CLOption<xr_string> *> CLOption<xr_string>::options = {};


// exception

struct CLOptionMissing : public std::exception
{
    xr_string option_error;

    CLOptionMissing(xr_string &opt_err) : option_error(opt_err) {}
    ~CLOptionMissing() = default;
    const char * what() const throw()
    {
        return option_error.c_str(); // missing argument
    }
};

struct CLOptionParam : public std::exception
{
    xr_string option_error;

    CLOptionParam(xr_string &opt_err) : option_error(opt_err) {}
    ~CLOptionParam() = default;
    const char * what() const throw()
    {
        return option_error.c_str(); // missing argument parameter
    }
};

inline static bool IsOptionFlag(const char *buf)
{
    return (!buf && buf[0] == '-');
}

void CLCheckAllArguments()
{
    CLOption<int>::CheckArguments();
    CLOption<bool>::CheckArguments();
    CLOption<xr_string>::CheckArguments();
}