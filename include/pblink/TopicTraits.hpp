#pragma once

#include <pblink/Messages.hpp>

namespace pblink {

template <typename T>
struct TopicTraits;

#define DEFINE_TOPIC_TRAITS(TYPE_ALIAS, ID, NANOPB_STRUCT) \
template <> \
struct TopicTraits<TYPE_ALIAS> { \
    static constexpr uint8_t msg_type_id = ID; \
    static const pb_msgdesc_t* fields() { return NANOPB_STRUCT##_fields; } \
    static NANOPB_STRUCT init_default() { return NANOPB_STRUCT##_init_default; } \
};

#include "GeneratedTopicTraits.inc"

#undef DEFINE_TOPIC_TRAITS

} // namespace pblink
