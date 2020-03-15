// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <stdint.h>

#include "az_iot_hub_client.h"
#include <az_precondition.h>
#include <az_precondition_internal.h>
#include <az_result.h>
#include <az_span.h>

#include <_az_cfg.h>

static const uint8_t telemetry_prop_delim = '?';
static const uint8_t telemetry_prop_separator = '&';
static const uint8_t telemetry_null_terminator = '\0';
static const az_span telemetry_topic_prefix = AZ_SPAN_LITERAL_FROM_STR("devices/");
static const az_span telemetry_topic_modules_mid = AZ_SPAN_LITERAL_FROM_STR("/modules/");
static const az_span telemetry_topic_suffix = AZ_SPAN_LITERAL_FROM_STR("/messages/events/");

AZ_NODISCARD az_result az_iot_hub_client_telemetry_publish_topic_get(
    az_iot_hub_client const* client,
    az_iot_hub_client_properties const* properties,
    az_span mqtt_topic,
    az_span* out_mqtt_topic)
{
  AZ_PRECONDITION_NOT_NULL(client);
  AZ_PRECONDITION_VALID_SPAN(mqtt_topic, 0, false);
  AZ_PRECONDITION_NOT_NULL(out_mqtt_topic);

  const az_span* user_agent = &(client->_internal.options.user_agent);
  const az_span* module_id = &(client->_internal.options.module_id);

  // Required topic parts
  int32_t required_size = az_span_length(telemetry_topic_prefix)
      + az_span_length(client->_internal.device_id) + az_span_length(telemetry_topic_suffix)
      + sizeof(telemetry_null_terminator);

  // Optional parts
  if (az_span_ptr(*module_id) != NULL)
  {
    required_size += az_span_length(telemetry_topic_modules_mid);
    required_size += az_span_length(*module_id);
  }

  if (properties != NULL)
  {
    required_size
        += az_span_length(properties->_internal.properties) + sizeof(telemetry_prop_delim);
  }

  if (az_span_ptr(*user_agent) != NULL)
  {
    required_size += az_span_length(*user_agent) + sizeof(telemetry_prop_delim);
  }

  // Only build topic if the span has the capacity
  if (az_span_capacity(mqtt_topic) < required_size)
  {
    return AZ_ERROR_INSUFFICIENT_SPAN_CAPACITY;
  }

#if 0
  // Build topic string
  AZ_RETURN_IF_FAILED(az_span_append(mqtt_topic, telemetry_topic_prefix, out_mqtt_topic));
  AZ_RETURN_IF_FAILED(az_span_append(*out_mqtt_topic, client->_internal.device_id, out_mqtt_topic));

  if (az_span_length(*module_id) != 0)
  {
    AZ_RETURN_IF_FAILED(
        az_span_append(*out_mqtt_topic, telemetry_topic_modules_mid, out_mqtt_topic));
    AZ_RETURN_IF_FAILED(az_span_append(*out_mqtt_topic, *module_id, out_mqtt_topic));
  }

  AZ_RETURN_IF_FAILED(az_span_append(*out_mqtt_topic, telemetry_topic_suffix, out_mqtt_topic));

  if (properties != NULL)
  {
    AZ_RETURN_IF_FAILED(
        az_span_append_uint8(*out_mqtt_topic, telemetry_prop_delim, out_mqtt_topic));
    AZ_RETURN_IF_FAILED(
        az_span_append(*out_mqtt_topic, properties->_internal.properties, out_mqtt_topic));
  }

  if (az_span_length(*user_agent) != 0)
  {
    AZ_RETURN_IF_FAILED(
        properties == NULL
            ? az_span_append_uint8(*out_mqtt_topic, telemetry_prop_delim, out_mqtt_topic)
            : az_span_append_uint8(*out_mqtt_topic, telemetry_prop_separator, out_mqtt_topic));
    AZ_RETURN_IF_FAILED(az_span_append(*out_mqtt_topic, *user_agent, out_mqtt_topic));
  }

  AZ_RETURN_IF_FAILED(
      az_span_append_uint8(*out_mqtt_topic, telemetry_null_terminator, out_mqtt_topic));
#else
  uint32_t index;
  uint8_t *buffer = az_span_ptr(mqtt_topic);
  
  memcpy(buffer, az_span_ptr(telemetry_topic_prefix), az_span_length(telemetry_topic_prefix));
  index = az_span_length(telemetry_topic_prefix);
  memcpy(&buffer[index], az_span_ptr(client->_internal.device_id), az_span_length(client->_internal.device_id));
  index += az_span_length(client->_internal.device_id);

  if (az_span_length(*module_id) != 0)
  {
    memcpy(&buffer[index], az_span_ptr(telemetry_topic_modules_mid), az_span_length(telemetry_topic_modules_mid));
    index += az_span_length(telemetry_topic_modules_mid);
  }
  
  memcpy(&buffer[index], az_span_ptr(telemetry_topic_suffix), az_span_length(telemetry_topic_suffix));
  index += az_span_length(telemetry_topic_suffix);

  if (properties != NULL)
  {
    buffer[index++] = telemetry_prop_delim;
    memcpy(&buffer[index], az_span_ptr(properties->_internal.properties), az_span_length(properties->_internal.properties));
    index += az_span_length(properties->_internal.properties);
  }

  if (az_span_length(*user_agent) != 0)
  {
    properties == NULL
            ? (buffer[index++] = telemetry_prop_delim)
            : (buffer[index++] = telemetry_prop_separator);
    memcpy(&buffer[index], az_span_ptr(*user_agent), az_span_length(*user_agent));
    index += az_span_length(*user_agent);
  }
  
  buffer[index++] = telemetry_null_terminator;

  *out_mqtt_topic = az_span_init(buffer, index, az_span_capacity(mqtt_topic));
#endif

  return AZ_OK;
}
