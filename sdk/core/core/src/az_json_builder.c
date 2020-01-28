// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "az_json_string_private.h"
#include "az_span_private.h"
#include <az_hex_internal.h>
#include <az_json_builder.h>
#include <az_json_string.h>
#include <az_span_action.h>
#include <az_span_reader.h>

#include <_az_cfg.h>

AZ_NODISCARD static az_result az_json_builder_append_str(az_json_builder * self, az_span value) {
  AZ_CONTRACT_ARG_NOT_NULL(self);

  az_span * json = &self->_internal.json;

  AZ_RETURN_IF_FAILED(az_span_append(*json, AZ_SPAN_FROM_STR("\""), json));
  AZ_RETURN_IF_FAILED(az_span_append(*json, value, json));
  AZ_RETURN_IF_FAILED(az_span_append(*json, AZ_SPAN_FROM_STR("\""), json));
  return AZ_OK;
}

AZ_NODISCARD az_result az_json_builder_write_span(az_json_builder * self, az_span value) {
  AZ_CONTRACT_ARG_NOT_NULL(self);

  az_span * json = &self->_internal.json;

  AZ_RETURN_IF_FAILED(az_span_append(*json, AZ_SPAN_FROM_STR("\""), json));

  for (int32_t i = 0; i < az_span_length(value); ++i) {
    az_result_byte const c = az_span_ptr(value)[i];

    // check if the character has to be escaped.
    {
      az_span const esc = az_json_esc_encode(c);
      AZ_RETURN_IF_FAILED(az_span_append(*json, esc, json));
    }
    // check if the character has to be escaped as a UNICODE escape sequence.
    if (0 <= c && c < 0x20) {
      uint8_t array[6] = {
        '\\',
        'u',
        '0',
        '0',
        az_number_to_upper_hex((uint8_t)(c / 16)),
        az_number_to_upper_hex((uint8_t)(c % 16)),
      };
      AZ_RETURN_IF_FAILED(az_span_append(*json, AZ_SPAN_FROM_INITIALIZED_BUFFER(array), json));
    }
  }
  return az_span_append(*json, AZ_SPAN_FROM_STR("\""), json);
}

AZ_NODISCARD az_result az_json_builder_write(az_json_builder * self, az_json_token value) {
  AZ_CONTRACT_ARG_NOT_NULL(self);
  az_span * json = &self->_internal.json;

  switch (value.kind) {
    case AZ_JSON_TOKEN_NULL: {
      self->_internal.need_comma = true;
      return az_span_append(*json, AZ_SPAN_FROM_STR("null"), json);
    }
    case AZ_JSON_TOKEN_BOOLEAN: {
      self->_internal.need_comma = true;
      return az_span_append(
          *json, value.data.boolean ? AZ_SPAN_FROM_STR("true") : AZ_SPAN_FROM_STR("false"), json);
    }
    case AZ_JSON_TOKEN_STRING: {
      self->_internal.need_comma = true;
      return az_json_builder_append_str(self, value.data.string);
    }
    case AZ_JSON_TOKEN_NUMBER: {
      self->_internal.need_comma = true;
      return az_span_append_double(*json, value.data.number, json);
    }
    case AZ_JSON_TOKEN_OBJECT: {
      self->_internal.need_comma = false;
      return az_span_append(*json, AZ_SPAN_FROM_STR("{"), json);
    }
    case AZ_JSON_TOKEN_ARRAY: {
      self->_internal.need_comma = false;
      return az_span_append(*json, AZ_SPAN_FROM_STR("["), json);
    }
    case AZ_JSON_TOKEN_SPAN: {
      self->_internal.need_comma = true;
      return az_json_builder_write_span(self, value.data.span);
    }
    default: { return AZ_ERROR_ARG; }
  }
}

AZ_NODISCARD static az_result az_json_builder_write_comma(az_json_builder * self) {
  AZ_CONTRACT_ARG_NOT_NULL(self);

  if (self->_internal.need_comma) {
    return az_span_append(self->_internal.json, AZ_SPAN_FROM_STR(","), &self->_internal.json);
  }
  return AZ_OK;
}

AZ_NODISCARD az_result
az_json_builder_write_object_member(az_json_builder * self, az_span name, az_json_token value) {
  AZ_CONTRACT_ARG_NOT_NULL(self);

  AZ_RETURN_IF_FAILED(az_json_builder_write_comma(self));
  AZ_RETURN_IF_FAILED(az_json_builder_append_str(self, name));
  AZ_RETURN_IF_FAILED(
      az_span_append(self->_internal.json, AZ_SPAN_FROM_STR(":"), &self->_internal.json));
  AZ_RETURN_IF_FAILED(az_json_builder_write(self, value));
  return AZ_OK;
}

AZ_NODISCARD static az_result az_json_builder_write_close(az_json_builder * self, az_span close) {
  AZ_CONTRACT_ARG_NOT_NULL(self);

  AZ_RETURN_IF_FAILED(az_span_append(self->_internal.json, close, &self->_internal.json));
  self->_internal.need_comma = true;
  return AZ_OK;
}

AZ_NODISCARD az_result az_json_builder_write_object_close(az_json_builder * self) {
  AZ_CONTRACT_ARG_NOT_NULL(self);

  return az_json_builder_write_close(self, AZ_SPAN_FROM_STR("}"));
}

AZ_NODISCARD az_result
az_json_builder_write_array_item(az_json_builder * self, az_json_token value) {
  AZ_CONTRACT_ARG_NOT_NULL(self);

  AZ_RETURN_IF_FAILED(az_json_builder_write_comma(self));
  AZ_RETURN_IF_FAILED(az_json_builder_write(self, value));
  return AZ_OK;
}

AZ_NODISCARD az_result az_json_builder_write_array_close(az_json_builder * self) {
  AZ_CONTRACT_ARG_NOT_NULL(self);

  return az_json_builder_write_close(self, AZ_SPAN_FROM_STR("]"));
}
