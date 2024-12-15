cmake_minimum_required(VERSION 3.1.3)
project(protobuf C CXX)

add_definitions(-DGOOGLE_PROTOBUF_CMAKE_BUILD)

if (MSVC)
  # Build with multiple processes
  add_definitions(/MP)
  # MSVC warning suppressions
  add_definitions(
    /wd4018 # 'expression' : signed/unsigned mismatch
    /wd4065 # switch statement contains 'default' but no 'case' labels
    /wd4146 # unary minus operator applied to unsigned type, result still unsigned
    /wd4244 # 'conversion' conversion from 'type1' to 'type2', possible loss of data
    /wd4251 # 'identifier' : class 'type' needs to have dll-interface to be used by clients of class 'type2'
    /wd4267 # 'var' : conversion from 'size_t' to 'type', possible loss of data
    /wd4305 # 'identifier' : truncation from 'type1' to 'type2'
    /wd4307 # 'operator' : integral constant overflow
    /wd4309 # 'conversion' : truncation of constant value
    /wd4334 # 'operator' : result of 32-bit shift implicitly converted to 64 bits (was 64-bit shift intended?)
    /wd4355 # 'this' : used in base member initializer list
    /wd4506 # no definition for inline function 'function'
    /wd4800 # 'type' : forcing value to bool 'true' or 'false' (performance warning)
    /wd4996 # The compiler encountered a deprecated declaration.
  )
  # Allow big object
  add_definitions(/bigobj)

  # Suppress linker warnings about files with no symbols defined.
  set(CMAKE_STATIC_LINKER_FLAGS /ignore:4221)
endif (MSVC)

find_package(Threads REQUIRED)
if (CMAKE_USE_PTHREADS_INIT)
	add_definitions(-DHAVE_PTHREAD)
endif (CMAKE_USE_PTHREADS_INIT)


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

#add_definitions(-DUNICODE -D_UNICODE)

file(GLOB LIBPROTOBUF_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/any.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/any.pb.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/api.pb.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/importer.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/parser.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/descriptor.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/descriptor.pb.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/descriptor_database.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/duration.pb.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/dynamic_message.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/empty.pb.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/extension_set_heavy.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/field_mask.pb.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/generated_message_reflection.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/generated_message_table_driven.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/io/gzip_stream.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/io/printer.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/io/tokenizer.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/io/zero_copy_stream_impl.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/map_field.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/message.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/reflection_ops.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/service.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/source_context.pb.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/struct.pb.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/stubs/mathlimits.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/stubs/substitute.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/text_format.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/timestamp.pb.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/type.pb.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/unknown_field_set.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/util/delimited_message_util.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/util/field_comparator.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/util/field_mask_util.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/util/internal/datapiece.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/util/internal/default_value_objectwriter.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/util/internal/error_listener.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/util/internal/field_mask_utility.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/util/internal/json_escaping.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/util/internal/json_objectwriter.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/util/internal/json_stream_parser.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/util/internal/object_writer.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/util/internal/proto_writer.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/util/internal/protostream_objectsource.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/util/internal/protostream_objectwriter.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/util/internal/type_info.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/util/internal/type_info_test_helper.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/util/internal/utility.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/util/json_util.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/util/message_differencer.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/util/time_util.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/util/type_resolver_util.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/wire_format.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/wrappers.pb.cc
)

file(GLOB LIBPROTOBUF_INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/any.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/any.pb.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/api.pb.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/importer.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/parser.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/descriptor.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/descriptor.pb.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/descriptor_database.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/duration.pb.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/dynamic_message.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/empty.pb.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/field_mask.pb.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/generated_message_reflection.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/io/gzip_stream.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/io/printer.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/io/tokenizer.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/io/zero_copy_stream_impl.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/map_field.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/message.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/reflection_ops.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/service.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/source_context.pb.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/struct.pb.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/stubs/mathlimits.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/stubs/substitute.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/text_format.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/timestamp.pb.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/type.pb.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/unknown_field_set.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/util/delimited_message_util.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/util/field_comparator.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/util/field_mask_util.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/util/internal/datapiece.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/util/internal/default_value_objectwriter.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/util/internal/error_listener.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/util/internal/field_mask_utility.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/util/internal/json_escaping.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/util/internal/json_objectwriter.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/util/internal/json_stream_parser.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/util/internal/object_writer.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/util/internal/proto_writer.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/util/internal/protostream_objectsource.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/util/internal/protostream_objectwriter.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/util/internal/type_info.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/util/internal/type_info_test_helper.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/util/internal/utility.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/util/json_util.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/util/message_differencer.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/util/time_util.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/util/type_resolver_util.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/wire_format.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/wrappers.pb.h
)

file(GLOB LIBPROTOBUF_LITE_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/arena.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/arenastring.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/extension_set.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/generated_message_table_driven_lite.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/generated_message_util.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/implicit_weak_message.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/io/coded_stream.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/io/zero_copy_stream.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/io/zero_copy_stream_impl_lite.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/message_lite.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/repeated_field.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/stubs/bytestream.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/stubs/common.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/stubs/int128.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/stubs/io_win32.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/stubs/status.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/stubs/statusor.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/stubs/stringpiece.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/stubs/stringprintf.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/stubs/structurally_valid.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/stubs/strutil.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/io/strtod.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/stubs/time.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/wire_format_lite.cc
)

file(GLOB LIBPROTOBUF_LITE_INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/arena.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/arenastring.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/extension_set.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/generated_message_util.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/implicit_weak_message.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/io/coded_stream.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/io/strtod.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/io/zero_copy_stream.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/io/zero_copy_stream_impl_lite.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/message_lite.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/repeated_field.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/stubs/bytestream.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/stubs/common.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/stubs/int128.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/stubs/once.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/stubs/status.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/stubs/statusor.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/stubs/stringpiece.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/stubs/stringprintf.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/stubs/strutil.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/stubs/time.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/wire_format_lite.h
)

add_library(protobuf-lite STATIC
	${LIBPROTOBUF_LITE_FILES}
	${LIBPROTOBUF_LITE_INCLUDES}
)

add_library(protobuf STATIC
	${LIBPROTOBUF_LITE_FILES}
	${LIBPROTOBUF_LITE_INCLUDES}
	${LIBPROTOBUF_FILES}
	${LIBPROTOBUF_INCLUDES}
)

target_link_libraries(protobuf ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(protobuf-lite ${CMAKE_THREAD_LIBS_INIT})

add_library(protobuf::libprotobuf-lite ALIAS protobuf-lite)
add_library(protobuf::libprotobuf ALIAS protobuf)

target_include_directories(protobuf PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(protobuf-lite PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(protobuf PROPERTIES FOLDER "third_party")
set_target_properties(protobuf-lite PROPERTIES FOLDER "third_party")


set(libprotoc_files
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/code_generator.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/command_line_interface.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/cpp/cpp_enum.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/cpp/cpp_enum_field.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/cpp/cpp_extension.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/cpp/cpp_field.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/cpp/cpp_file.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/cpp/cpp_generator.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/cpp/cpp_helpers.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/cpp/cpp_map_field.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/cpp/cpp_message.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/cpp/cpp_message_field.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/cpp/cpp_padding_optimizer.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/cpp/cpp_primitive_field.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/cpp/cpp_service.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/cpp/cpp_string_field.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/csharp/csharp_doc_comment.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/csharp/csharp_enum.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/csharp/csharp_enum_field.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/csharp/csharp_field_base.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/csharp/csharp_generator.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/csharp/csharp_helpers.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/csharp/csharp_map_field.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/csharp/csharp_message.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/csharp/csharp_message_field.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/csharp/csharp_primitive_field.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/csharp/csharp_reflection_class.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/csharp/csharp_repeated_enum_field.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/csharp/csharp_repeated_message_field.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/csharp/csharp_repeated_primitive_field.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/csharp/csharp_source_generator_base.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/csharp/csharp_wrapper_field.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_context.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_doc_comment.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_enum.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_enum_field.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_enum_field_lite.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_enum_lite.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_extension.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_extension_lite.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_field.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_file.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_generator.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_generator_factory.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_helpers.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_lazy_message_field.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_lazy_message_field_lite.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_map_field.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_map_field_lite.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_message.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_message_builder.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_message_builder_lite.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_message_field.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_message_field_lite.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_message_lite.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_name_resolver.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_primitive_field.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_primitive_field_lite.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_service.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_shared_code_generator.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_string_field.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_string_field_lite.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/js/js_generator.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/js/well_known_types_embed.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/objectivec/objectivec_enum.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/objectivec/objectivec_enum_field.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/objectivec/objectivec_extension.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/objectivec/objectivec_field.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/objectivec/objectivec_file.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/objectivec/objectivec_generator.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/objectivec/objectivec_helpers.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/objectivec/objectivec_map_field.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/objectivec/objectivec_message.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/objectivec/objectivec_message_field.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/objectivec/objectivec_oneof.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/objectivec/objectivec_primitive_field.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/php/php_generator.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/plugin.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/plugin.pb.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/python/python_generator.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/ruby/ruby_generator.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/subprocess.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/zip_writer.cc
)

set(libprotoc_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/cpp/cpp_enum.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/cpp/cpp_enum_field.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/cpp/cpp_extension.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/cpp/cpp_field.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/cpp/cpp_file.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/cpp/cpp_helpers.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/cpp/cpp_map_field.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/cpp/cpp_message.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/cpp/cpp_message_field.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/cpp/cpp_message_layout_helper.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/cpp/cpp_options.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/cpp/cpp_padding_optimizer.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/cpp/cpp_primitive_field.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/cpp/cpp_service.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/cpp/cpp_string_field.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/csharp/csharp_doc_comment.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/csharp/csharp_enum.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/csharp/csharp_enum_field.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/csharp/csharp_field_base.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/csharp/csharp_helpers.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/csharp/csharp_map_field.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/csharp/csharp_message.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/csharp/csharp_message_field.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/csharp/csharp_options.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/csharp/csharp_primitive_field.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/csharp/csharp_reflection_class.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/csharp/csharp_repeated_enum_field.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/csharp/csharp_repeated_message_field.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/csharp/csharp_repeated_primitive_field.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/csharp/csharp_source_generator_base.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/csharp/csharp_wrapper_field.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_context.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_doc_comment.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_enum.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_enum_field.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_enum_field_lite.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_enum_lite.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_extension.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_extension_lite.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_field.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_file.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_generator_factory.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_helpers.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_lazy_message_field.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_lazy_message_field_lite.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_map_field.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_map_field_lite.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_message.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_message_builder.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_message_builder_lite.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_message_field.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_message_field_lite.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_message_lite.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_name_resolver.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_options.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_primitive_field.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_primitive_field_lite.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_service.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_shared_code_generator.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_string_field.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/java/java_string_field_lite.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/objectivec/objectivec_enum.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/objectivec/objectivec_enum_field.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/objectivec/objectivec_extension.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/objectivec/objectivec_field.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/objectivec/objectivec_file.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/objectivec/objectivec_helpers.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/objectivec/objectivec_map_field.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/objectivec/objectivec_message.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/objectivec/objectivec_message_field.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/objectivec/objectivec_oneof.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/objectivec/objectivec_primitive_field.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/subprocess.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/zip_writer.h
)

option(ENABLE_BUILC_PROTOC "build protobuf protoc" OFF)

if (ENABLE_BUILC_PROTOC)
	add_library(protoc STATIC ${libprotoc_files} ${libprotoc_headers})
	target_link_libraries(protoc protobuf)
	add_library(protobuf::protoc ALIAS protoc)

	file(GLOB PROTOC_FILES
		${CMAKE_CURRENT_SOURCE_DIR}/src/google/protobuf/compiler/main.cc
	)

	add_executable(protoc ${PROTOC_FILES})
	target_link_libraries(protoc protobuf protoc)
	add_executable(protobuf::protoc ALIAS protoc)

	set_target_properties(protoc PROPERTIES FOLDER "third_party")
	set_target_properties(protoc PROPERTIES FOLDER "third_party")
endif()
