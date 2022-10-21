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
struct LIBPROTOC_EXPORT TableStruct {
  static const ::google::protobuf::internal::ParseTableField entries[];
  static const ::google::protobuf::internal::AuxillaryParseTableField aux[];
  static const ::google::protobuf::internal::ParseTable schema[];
  static const ::google::protobuf::uint32 offsets[];
  static const ::google::protobuf::internal::FieldMetadata field_metadata[];
  static const ::google::protobuf::internal::SerializationTable serialization_table[];
  static void InitDefaultsImpl();
};
void LIBPROTOC_EXPORT AddDescriptors();
void LIBPROTOC_EXPORT InitDefaults();
}  // namespace protobuf_define_2eproto

enum LP_CM_MSG_ID {
  CM_MSG_NONE = 0,
  CM_BEGAN = 10000,
  CM_LOGIN = 10001,
  CM_ENTER_ROOM = 10002,
  CM_CREATE_ROLE = 10003,
  CM_BEGIN_MATCH = 10004,
  CM_STOP_MATCH = 10005,
  CM_ENTER_BATTLE_SCENE = 10006,
  CM_PLAYER_OPERATION = 10007,
  CM_FIX_FRAME = 10008,
  CM_ENTER_GAME = 10009,
  CM_SEARCH_PLAYER = 10010,
  CM_ADD_FRIEND = 10011,
  CM_OPT_FRIEND_APPLY = 10012,
  CM_DELETE_FRIEND = 10013,
  CM_SYNC_RELATION_INFO = 10014,
  CM_END = 15000,
  LP_CM_MSG_ID_INT_MIN_SENTINEL_DO_NOT_USE_ = ::google::protobuf::kint32min,
  LP_CM_MSG_ID_INT_MAX_SENTINEL_DO_NOT_USE_ = ::google::protobuf::kint32max
};
LIBPROTOC_EXPORT bool LP_CM_MSG_ID_IsValid(int value);
const LP_CM_MSG_ID LP_CM_MSG_ID_MIN = CM_MSG_NONE;
const LP_CM_MSG_ID LP_CM_MSG_ID_MAX = CM_END;
const int LP_CM_MSG_ID_ARRAYSIZE = LP_CM_MSG_ID_MAX + 1;

LIBPROTOC_EXPORT const ::google::protobuf::EnumDescriptor* LP_CM_MSG_ID_descriptor();
inline const ::std::string& LP_CM_MSG_ID_Name(LP_CM_MSG_ID value) {
  return ::google::protobuf::internal::NameOfEnum(
    LP_CM_MSG_ID_descriptor(), value);
}
inline bool LP_CM_MSG_ID_Parse(
    const ::std::string& name, LP_CM_MSG_ID* value) {
  return ::google::protobuf::internal::ParseNamedEnum<LP_CM_MSG_ID>(
    LP_CM_MSG_ID_descriptor(), name, value);
}
enum LP_SM_MSG_ID {
  SM_MSG_NONE = 0,
  SM_BEGIN = 15000,
  SM_LOGIN_RES = 15001,
  SM_ENTER_ROOM = 15002,
  SM_CREATE_ROLE = 15003,
  SM_MATCH_STATE = 15004,
  SM_ENTER_BATTLE = 15005,
  SM_PLAYER_OPERATION = 15006,
  SM_OBJECT_INFO = 15007,
  SM_SELF_ROLE_INFO = 15008,
  SM_OPERATION_SIZE = 15009,
  SM_OBJECT_SIZE = 15010,
  SM_OLD_BATTLE_INFO = 15011,
  SM_FIX_FRAME = 15012,
  SM_RELATION_INFO = 15013,
  SM_END = 20000,
  LP_SM_MSG_ID_INT_MIN_SENTINEL_DO_NOT_USE_ = ::google::protobuf::kint32min,
  LP_SM_MSG_ID_INT_MAX_SENTINEL_DO_NOT_USE_ = ::google::protobuf::kint32max
};
LIBPROTOC_EXPORT bool LP_SM_MSG_ID_IsValid(int value);
const LP_SM_MSG_ID LP_SM_MSG_ID_MIN = SM_MSG_NONE;
const LP_SM_MSG_ID LP_SM_MSG_ID_MAX = SM_END;
const int LP_SM_MSG_ID_ARRAYSIZE = LP_SM_MSG_ID_MAX + 1;

LIBPROTOC_EXPORT const ::google::protobuf::EnumDescriptor* LP_SM_MSG_ID_descriptor();
inline const ::std::string& LP_SM_MSG_ID_Name(LP_SM_MSG_ID value) {
  return ::google::protobuf::internal::NameOfEnum(
    LP_SM_MSG_ID_descriptor(), value);
}
inline bool LP_SM_MSG_ID_Parse(
    const ::std::string& name, LP_SM_MSG_ID* value) {
  return ::google::protobuf::internal::ParseNamedEnum<LP_SM_MSG_ID>(
    LP_SM_MSG_ID_descriptor(), name, value);
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

template <> struct is_proto_enum< ::LPMsg::LP_CM_MSG_ID> : ::google::protobuf::internal::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor< ::LPMsg::LP_CM_MSG_ID>() {
  return ::LPMsg::LP_CM_MSG_ID_descriptor();
}
template <> struct is_proto_enum< ::LPMsg::LP_SM_MSG_ID> : ::google::protobuf::internal::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor< ::LPMsg::LP_SM_MSG_ID>() {
  return ::LPMsg::LP_SM_MSG_ID_descriptor();
}

}  // namespace protobuf
}  // namespace google

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_define_2eproto__INCLUDED
