#include <rapidjson/rapidjson.h>

RAPIDJSON_NAMESPACE_BEGIN

const char* GetVersion() RAPIDJSON_NOEXCEPT {
    return RAPIDJSON_VERSION_STRING;
}

RAPIDJSON_NAMESPACE_END
