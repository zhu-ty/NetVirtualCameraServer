#ifndef COLORMOD
#define COLORMOD
#include <ostream>
namespace Colormod
{
enum Code
{
    FG_RED      = 31,
    FG_GREEN    = 32,
    FG_YELLOW   = 33,
    FG_BLUE     = 34,
    FG_MAGENTA  = 35,
    FG_CYAN     = 36,
    FG_WHITE    = 37,
    FG_DEFAULT  = 39,
    BG_RED      = 41,
    BG_GREEN    = 42,
    BG_YELLOW   = 43,
    BG_BLUE     = 44,
    BG_MAGENTA  = 45,
    BG_CYAN     = 46,
    BG_WHITE    = 47,
    BG_DEFAULT  = 49,
};
class Modifier
{
    Code code;
public:
    Modifier(Code pCode) : code(pCode) {}
    friend std::ostream&
    operator<<(std::ostream& os, const Modifier& mod)
    {
        return os << "\033[" << mod.code << "m";
    }
};

static Modifier red(Colormod::FG_RED);
static Modifier def(Colormod::FG_DEFAULT);
static Modifier white(Colormod::FG_WHITE);
static Modifier blue(Colormod::FG_BLUE);
static Modifier yellow(Colormod::FG_YELLOW);
static Modifier green(Colormod::FG_GREEN);
}

#endif


//usage:
//Color::Modifier red(Color::FG_RED);
