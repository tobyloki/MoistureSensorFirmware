/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <esp_log.h>
#include <stdlib.h>
#include <string.h>

#include <device.h>
#include <esp_matter.h>
#include <led_driver.h>

#include <app_priv.h>

using namespace chip::app::Clusters;
using namespace esp_matter;

static const char *TAG = "app_driver";
// extern uint16_t temperature_endpoint_id;
// extern uint16_t pressure_endpoint_id;
// extern uint16_t humidity_endpoint_id;

/* Do any conversions/remapping for the actual value here */

static void app_driver_button_toggle_cb(void *arg, void *data)
{
    ESP_LOGI(TAG, "Toggle button pressed");
}

esp_err_t app_driver_attribute_update(app_driver_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id,
                                      uint32_t attribute_id, esp_matter_attr_val_t *val)
{
    esp_err_t err = ESP_OK;
    return err;
}

// esp_err_t app_driver_temperature_set_defaults(uint16_t endpoint_id)
// {
//     esp_err_t err = ESP_OK;
//     void *priv_data = endpoint::get_priv_data(endpoint_id);
//     // temperature_driver_handle_t handle = (temperature_driver_handle_t)priv_data;
//     node_t *node = node::get();
//     endpoint_t *endpoint = endpoint::get(node, endpoint_id);
//     cluster_t *cluster = NULL;
//     attribute_t *attribute = NULL;
//     esp_matter_attr_val_t val = esp_matter_invalid(NULL);

//     /* Get temperature */
//     cluster = cluster::get(endpoint, TemperatureMeasurement::Id);
//     attribute = attribute::get(cluster, TemperatureMeasurement::Attributes::MeasuredValue::Id);
//     attribute::get_val(attribute, &val);
//     // err |= app_driver_light_set_brightness(handle, &val);
//     // print value
//     ESP_LOGI(TAG, "Temperature: %d", val.val.i16);

//     return err;
// }

// esp_err_t app_driver_pressure_set_defaults(uint16_t endpoint_id)
// {
//     esp_err_t err = ESP_OK;
//     void *priv_data = endpoint::get_priv_data(endpoint_id);
//     // pressure_driver_handle_t handle = (pressure_driver_handle_t)priv_data;
//     node_t *node = node::get();
//     endpoint_t *endpoint = endpoint::get(node, endpoint_id);
//     cluster_t *cluster = NULL;
//     attribute_t *attribute = NULL;
//     esp_matter_attr_val_t val = esp_matter_invalid(NULL);

//     /* Get pressure */
//     cluster = cluster::get(endpoint, PressureMeasurement::Id);
//     attribute = attribute::get(cluster, PressureMeasurement::Attributes::MeasuredValue::Id);
//     attribute::get_val(attribute, &val);
//     // err |= app_driver_light_set_brightness(handle, &val);
//     // print value
//     ESP_LOGI(TAG, "Pressure: %d", val.val.i16);

//     return err;
// }

// esp_err_t app_driver_humidity_set_defaults(uint16_t endpoint_id)
// {
//     esp_err_t err = ESP_OK;
//     void *priv_data = endpoint::get_priv_data(endpoint_id);
//     // humidity_driver_handle_t handle = (humidity_driver_handle_t)priv_data;
//     node_t *node = node::get();
//     endpoint_t *endpoint = endpoint::get(node, endpoint_id);
//     cluster_t *cluster = NULL;
//     attribute_t *attribute = NULL;
//     esp_matter_attr_val_t val = esp_matter_invalid(NULL);

//     /* Get humidity */
//     cluster = cluster::get(endpoint, RelativeHumidityMeasurement::Id);
//     attribute = attribute::get(cluster, RelativeHumidityMeasurement::Attributes::MeasuredValue::Id);
//     attribute::get_val(attribute, &val);
//     // err |= app_driver_light_set_brightness(handle, &val);
//     // print value
//     ESP_LOGI(TAG, "Humidity: %d", val.val.u16);

//     /* Get device id */
//     cluster = cluster::get(endpoint, RelativeHumidityMeasurement::Id);
//     attribute = attribute::get(cluster, RelativeHumidityMeasurement::Attributes::MinMeasuredValue::Id);
//     attribute::get_val(attribute, &val);
//     // print value
//     ESP_LOGI(TAG, "Device id: %d", val.val.u16);

//     /* Get battery percentage */
//     cluster = cluster::get(endpoint, RelativeHumidityMeasurement::Id);
//     attribute = attribute::get(cluster, RelativeHumidityMeasurement::Attributes::MaxMeasuredValue::Id);
//     attribute::get_val(attribute, &val);
//     // print value
//     ESP_LOGI(TAG, "Battery: %d", val.val.u16);

//     return err;
// }

app_driver_handle_t app_driver_sensor_init()
{
    /* Initialize led */
    led_driver_config_t config = led_driver_get_config();
    led_driver_handle_t handle = led_driver_init(&config);
    return (app_driver_handle_t)handle;
}

app_driver_handle_t app_driver_button_init()
{
    /* Initialize button */
    button_config_t config = button_driver_get_config();
    button_handle_t handle = iot_button_create(&config);
    iot_button_register_cb(handle, BUTTON_PRESS_DOWN, app_driver_button_toggle_cb, NULL);
    return (app_driver_handle_t)handle;
}
