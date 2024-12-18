/*
 * Copyright (c) 2021 PaddlePaddle Authors. All Rights Reserved.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: google/protobuf/field_mask.proto

#ifndef PROTOBUF_google_2fprotobuf_2ffield_5fmask_2eproto__INCLUDED
#define PROTOBUF_google_2fprotobuf_2ffield_5fmask_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 3001000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 3001000 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)

namespace google {
namespace protobuf {

// Internal implementation detail -- do not call these.
void LIBPROTOBUF_EXPORT protobuf_AddDesc_google_2fprotobuf_2ffield_5fmask_2eproto();
void LIBPROTOBUF_EXPORT protobuf_InitDefaults_google_2fprotobuf_2ffield_5fmask_2eproto();
void protobuf_AssignDesc_google_2fprotobuf_2ffield_5fmask_2eproto();
void protobuf_ShutdownFile_google_2fprotobuf_2ffield_5fmask_2eproto();

class FieldMask;

// ===================================================================

class LIBPROTOBUF_EXPORT FieldMask : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:google.protobuf.FieldMask) */ {
 public:
  FieldMask();
  virtual ~FieldMask();

  FieldMask(const FieldMask& from);

  inline FieldMask& operator=(const FieldMask& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const FieldMask& default_instance();

  static const FieldMask* internal_default_instance();

  void Swap(FieldMask* other);

  // implements Message ----------------------------------------------

  inline FieldMask* New() const { return New(NULL); }

  FieldMask* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const FieldMask& from);
  void MergeFrom(const FieldMask& from);
  void Clear();
  bool IsInitialized() const;

  size_t ByteSizeLong() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const {
    return InternalSerializeWithCachedSizesToArray(false, output);
  }
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(FieldMask* other);
  void UnsafeMergeFrom(const FieldMask& from);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // repeated string paths = 1;
  int paths_size() const;
  void clear_paths();
  static const int kPathsFieldNumber = 1;
  const ::std::string& paths(int index) const;
  ::std::string* mutable_paths(int index);
  void set_paths(int index, const ::std::string& value);
  void set_paths(int index, const char* value);
  void set_paths(int index, const char* value, size_t size);
  ::std::string* add_paths();
  void add_paths(const ::std::string& value);
  void add_paths(const char* value);
  void add_paths(const char* value, size_t size);
  const ::google::protobuf::RepeatedPtrField< ::std::string>& paths() const;
  ::google::protobuf::RepeatedPtrField< ::std::string>* mutable_paths();

  // @@protoc_insertion_point(class_scope:google.protobuf.FieldMask)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::RepeatedPtrField< ::std::string> paths_;
  mutable int _cached_size_;
  friend void LIBPROTOBUF_EXPORT protobuf_InitDefaults_google_2fprotobuf_2ffield_5fmask_2eproto_impl();
  friend void LIBPROTOBUF_EXPORT protobuf_AddDesc_google_2fprotobuf_2ffield_5fmask_2eproto_impl();
  friend void protobuf_AssignDesc_google_2fprotobuf_2ffield_5fmask_2eproto();
  friend void protobuf_ShutdownFile_google_2fprotobuf_2ffield_5fmask_2eproto();

  void InitAsDefaultInstance();
};
extern ::google::protobuf::internal::ExplicitlyConstructed<FieldMask> FieldMask_default_instance_;

// ===================================================================


// ===================================================================

#if !PROTOBUF_INLINE_NOT_IN_HEADERS
// FieldMask

// repeated string paths = 1;
inline int FieldMask::paths_size() const {
  return paths_.size();
}
inline void FieldMask::clear_paths() {
  paths_.Clear();
}
inline const ::std::string& FieldMask::paths(int index) const {
  // @@protoc_insertion_point(field_get:google.protobuf.FieldMask.paths)
  return paths_.Get(index);
}
inline ::std::string* FieldMask::mutable_paths(int index) {
  // @@protoc_insertion_point(field_mutable:google.protobuf.FieldMask.paths)
  return paths_.Mutable(index);
}
inline void FieldMask::set_paths(int index, const ::std::string& value) {
  // @@protoc_insertion_point(field_set:google.protobuf.FieldMask.paths)
  paths_.Mutable(index)->assign(value);
}
inline void FieldMask::set_paths(int index, const char* value) {
  paths_.Mutable(index)->assign(value);
  // @@protoc_insertion_point(field_set_char:google.protobuf.FieldMask.paths)
}
inline void FieldMask::set_paths(int index, const char* value, size_t size) {
  paths_.Mutable(index)->assign(
    reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_set_pointer:google.protobuf.FieldMask.paths)
}
inline ::std::string* FieldMask::add_paths() {
  // @@protoc_insertion_point(field_add_mutable:google.protobuf.FieldMask.paths)
  return paths_.Add();
}
inline void FieldMask::add_paths(const ::std::string& value) {
  paths_.Add()->assign(value);
  // @@protoc_insertion_point(field_add:google.protobuf.FieldMask.paths)
}
inline void FieldMask::add_paths(const char* value) {
  paths_.Add()->assign(value);
  // @@protoc_insertion_point(field_add_char:google.protobuf.FieldMask.paths)
}
inline void FieldMask::add_paths(const char* value, size_t size) {
  paths_.Add()->assign(reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_add_pointer:google.protobuf.FieldMask.paths)
}
inline const ::google::protobuf::RepeatedPtrField< ::std::string>&
FieldMask::paths() const {
  // @@protoc_insertion_point(field_list:google.protobuf.FieldMask.paths)
  return paths_;
}
inline ::google::protobuf::RepeatedPtrField< ::std::string>*
FieldMask::mutable_paths() {
  // @@protoc_insertion_point(field_mutable_list:google.protobuf.FieldMask.paths)
  return &paths_;
}

inline const FieldMask* FieldMask::internal_default_instance() {
  return &FieldMask_default_instance_.get();
}
#endif  // !PROTOBUF_INLINE_NOT_IN_HEADERS

// @@protoc_insertion_point(namespace_scope)

}  // namespace protobuf
}  // namespace google

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_google_2fprotobuf_2ffield_5fmask_2eproto__INCLUDED
