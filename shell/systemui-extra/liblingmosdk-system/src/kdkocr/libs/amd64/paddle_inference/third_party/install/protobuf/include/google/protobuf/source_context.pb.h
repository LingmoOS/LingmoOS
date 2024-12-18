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
// source: google/protobuf/source_context.proto

#ifndef PROTOBUF_google_2fprotobuf_2fsource_5fcontext_2eproto__INCLUDED
#define PROTOBUF_google_2fprotobuf_2fsource_5fcontext_2eproto__INCLUDED

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
void LIBPROTOBUF_EXPORT protobuf_AddDesc_google_2fprotobuf_2fsource_5fcontext_2eproto();
void LIBPROTOBUF_EXPORT protobuf_InitDefaults_google_2fprotobuf_2fsource_5fcontext_2eproto();
void protobuf_AssignDesc_google_2fprotobuf_2fsource_5fcontext_2eproto();
void protobuf_ShutdownFile_google_2fprotobuf_2fsource_5fcontext_2eproto();

class SourceContext;

// ===================================================================

class LIBPROTOBUF_EXPORT SourceContext : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:google.protobuf.SourceContext) */ {
 public:
  SourceContext();
  virtual ~SourceContext();

  SourceContext(const SourceContext& from);

  inline SourceContext& operator=(const SourceContext& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const SourceContext& default_instance();

  static const SourceContext* internal_default_instance();

  void Swap(SourceContext* other);

  // implements Message ----------------------------------------------

  inline SourceContext* New() const { return New(NULL); }

  SourceContext* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const SourceContext& from);
  void MergeFrom(const SourceContext& from);
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
  void InternalSwap(SourceContext* other);
  void UnsafeMergeFrom(const SourceContext& from);
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

  // optional string file_name = 1;
  void clear_file_name();
  static const int kFileNameFieldNumber = 1;
  const ::std::string& file_name() const;
  void set_file_name(const ::std::string& value);
  void set_file_name(const char* value);
  void set_file_name(const char* value, size_t size);
  ::std::string* mutable_file_name();
  ::std::string* release_file_name();
  void set_allocated_file_name(::std::string* file_name);

  // @@protoc_insertion_point(class_scope:google.protobuf.SourceContext)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::internal::ArenaStringPtr file_name_;
  mutable int _cached_size_;
  friend void LIBPROTOBUF_EXPORT protobuf_InitDefaults_google_2fprotobuf_2fsource_5fcontext_2eproto_impl();
  friend void LIBPROTOBUF_EXPORT protobuf_AddDesc_google_2fprotobuf_2fsource_5fcontext_2eproto_impl();
  friend void protobuf_AssignDesc_google_2fprotobuf_2fsource_5fcontext_2eproto();
  friend void protobuf_ShutdownFile_google_2fprotobuf_2fsource_5fcontext_2eproto();

  void InitAsDefaultInstance();
};
extern ::google::protobuf::internal::ExplicitlyConstructed<SourceContext> SourceContext_default_instance_;

// ===================================================================


// ===================================================================

#if !PROTOBUF_INLINE_NOT_IN_HEADERS
// SourceContext

// optional string file_name = 1;
inline void SourceContext::clear_file_name() {
  file_name_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& SourceContext::file_name() const {
  // @@protoc_insertion_point(field_get:google.protobuf.SourceContext.file_name)
  return file_name_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void SourceContext::set_file_name(const ::std::string& value) {
  
  file_name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:google.protobuf.SourceContext.file_name)
}
inline void SourceContext::set_file_name(const char* value) {
  
  file_name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:google.protobuf.SourceContext.file_name)
}
inline void SourceContext::set_file_name(const char* value, size_t size) {
  
  file_name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:google.protobuf.SourceContext.file_name)
}
inline ::std::string* SourceContext::mutable_file_name() {
  
  // @@protoc_insertion_point(field_mutable:google.protobuf.SourceContext.file_name)
  return file_name_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* SourceContext::release_file_name() {
  // @@protoc_insertion_point(field_release:google.protobuf.SourceContext.file_name)
  
  return file_name_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void SourceContext::set_allocated_file_name(::std::string* file_name) {
  if (file_name != NULL) {
    
  } else {
    
  }
  file_name_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), file_name);
  // @@protoc_insertion_point(field_set_allocated:google.protobuf.SourceContext.file_name)
}

inline const SourceContext* SourceContext::internal_default_instance() {
  return &SourceContext_default_instance_.get();
}
#endif  // !PROTOBUF_INLINE_NOT_IN_HEADERS

// @@protoc_insertion_point(namespace_scope)

}  // namespace protobuf
}  // namespace google

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_google_2fprotobuf_2fsource_5fcontext_2eproto__INCLUDED
