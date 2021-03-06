// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: define.proto

#ifndef PROTOBUF_define_2eproto__INCLUDED
#define PROTOBUF_define_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 3004000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 3004000 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_table_driven.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/generated_enum_reflection.h>
// @@protoc_insertion_point(includes)
namespace LPMsg {
}  // namespace LPMsg

namespace LPMsg {

namespace protobuf_define_2eproto {
// Internal implementation detail -- do not call these.
struct TableStruct {
  static const ::google::protobuf::internal::ParseTableField entries[];
  static const ::google::protobuf::internal::AuxillaryParseTableField aux[];
  static const ::google::protobuf::internal::ParseTable schema[];
  static const ::google::protobuf::uint32 offsets[];
  static const ::google::protobuf::internal::FieldMetadata field_metadata[];
  static const ::google::protobuf::internal::SerializationTable serialization_table[];
  static void InitDefaultsImpl();
};
void AddDescriptors();
void InitDefaults();
}  // namespace protobuf_define_2eproto

enum LP_MSG_ID {
  N_NONE = 0,
  N_BEGAN = 10000,
  N_REQ_LOGIN = 10001,
  N_ACK_LOGIN_RES = 10002,
  LP_MSG_ID_INT_MIN_SENTINEL_DO_NOT_USE_ = ::google::protobuf::kint32min,
  LP_MSG_ID_INT_MAX_SENTINEL_DO_NOT_USE_ = ::google::protobuf::kint32max
};
bool LP_MSG_ID_IsValid(int value);
const LP_MSG_ID LP_MSG_ID_MIN = N_NONE;
const LP_MSG_ID LP_MSG_ID_MAX = N_ACK_LOGIN_RES;
const int LP_MSG_ID_ARRAYSIZE = LP_MSG_ID_MAX + 1;

const ::google::protobuf::EnumDescriptor* LP_MSG_ID_descriptor();
inline const ::std::string& LP_MSG_ID_Name(LP_MSG_ID value) {
  return ::google::protobuf::internal::NameOfEnum(
    LP_MSG_ID_descriptor(), value);
}
inline bool LP_MSG_ID_Parse(
    const ::std::string& name, LP_MSG_ID* value) {
  return ::google::protobuf::internal::ParseNamedEnum<LP_MSG_ID>(
    LP_MSG_ID_descriptor(), name, value);
}
// ===================================================================


// ===================================================================


// ===================================================================

#if !PROTOBUF_INLINE_NOT_IN_HEADERS
#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__
#endif  // !PROTOBUF_INLINE_NOT_IN_HEADERS

// @@protoc_insertion_point(namespace_scope)


}  // namespace LPMsg

namespace google {
namespace protobuf {

template <> struct is_proto_enum< ::LPMsg::LP_MSG_ID> : ::google::protobuf::internal::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor< ::LPMsg::LP_MSG_ID>() {
  return ::LPMsg::LP_MSG_ID_descriptor();
}

}  // namespace protobuf
}  // namespace google

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_define_2eproto__INCLUDED
