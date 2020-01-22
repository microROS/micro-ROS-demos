#include <rcl/rcl.h>
#include <rcl/parameter_service.h>
#include <rcl/error_handling.h>

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){printf("Failed status on line %d: %d. Aborting.\n",__LINE__,(int)temp_rc); return 1;}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){printf("Failed status on line %d: %d. Continuing.\n",__LINE__,(int)temp_rc); break;}}

int main(int argc, const char * const * argv)
{
  /****************************************************************************
   * Initialization.
   ****************************************************************************/
  rcl_init_options_t options = rcl_get_zero_initialized_init_options();
  RCCHECK(rcl_init_options_init(&options, rcl_get_default_allocator()))

  rcl_context_t context = rcl_get_zero_initialized_context();
  RCCHECK(rcl_init(argc, argv, &options, &context))

  rcl_node_options_t node_ops = rcl_node_get_default_options();
  rcl_node_t node = rcl_get_zero_initialized_node();
  RCCHECK(rcl_node_init(&node, "parameter_client_rcl", "", &context, &node_ops))

  rcl_parameter_service_t parameter_service = rcl_get_zero_initialized_parameter_service();
  const rcl_parameter_service_options_t ps_options = rcl_parameter_service_get_default_options();
  RCCHECK(rcl_parameter_service_init(&parameter_service, &node, &ps_options))

  rcl_wait_set_t wait_set = rcl_get_zero_initialized_wait_set();
  RCCHECK(rcl_wait_set_init(&wait_set, 0, 0, 0, 0, RCL_NUMBER_OF_PARAMETER_ACTIONS, 0, &context, rcl_get_default_allocator()))

  /****************************************************************************
   * Parameters.
   ****************************************************************************/
  const size_t num_params = 4;
  bool bool_parameter;
  int64_t int_parameter;
  double double_parameter;
  char string_parameter[64];

  /****************************************************************************
   * Manage service.
   ****************************************************************************/
  do {
    RCSOFTCHECK(rcl_wait_set_clear(&wait_set))
    RCSOFTCHECK(rcl_wait_set_add_parameter_service(&wait_set, &parameter_service))
    RCSOFTCHECK(rcl_wait(&wait_set, RCL_MS_TO_NS(50)))

    rcl_param_action_t action;
    rmw_request_id_t request_header;
    do {
      RCSOFTCHECK(rcl_parameter_service_get_pending_action(&wait_set, &parameter_service, &action))
      switch (action)
      {
        case RCL_LIST_PARAMETERS:
        {
          rcl_interfaces__msg__ListParametersResult list_parameters_result;
          rosidl_generator_c__String__Sequence prefixes;
          uint64_t depth;

          rcl_interfaces__msg__ListParametersResult__init(&list_parameters_result);
          rosidl_generator_c__String__Sequence__init(&list_parameters_result.names, num_params);
          rosidl_generator_c__String__Sequence__init(&list_parameters_result.prefixes, num_params);
          rosidl_generator_c__String__assign(&list_parameters_result.names.data[0], "bool_param");
          rosidl_generator_c__String__assign(&list_parameters_result.names.data[1], "int_param");
          rosidl_generator_c__String__assign(&list_parameters_result.names.data[2], "double_param");
          rosidl_generator_c__String__assign(&list_parameters_result.names.data[3], "string_param");
          rosidl_generator_c__String__Sequence__init(&prefixes, 0);

          RCSOFTCHECK(rcl_parameter_service_take_list_request(&parameter_service, &request_header, &prefixes, &depth))
          RCSOFTCHECK(rcl_parameter_service_send_list_response(&parameter_service, &request_header, &list_parameters_result))
          break;
        }
        case RCL_GET_PARAMETERS:
        {
          rcl_interfaces__msg__ParameterValue__Sequence parameter_values;
          rosidl_generator_c__String__Sequence * get_request;

          get_request = rcl_parameter_service_take_get_request(&parameter_service, &request_header);
          if (!get_request) {
            printf("Error taking get request\n");
            break;
          }

          rcl_interfaces__msg__ParameterValue__Sequence__init(&parameter_values, get_request->size);
          for (size_t i = 0; i < get_request->size; ++i) 
          {
            if (0 == strcmp(get_request->data[i].data, "bool_param")) 
            {
              RCSOFTCHECK(rcl_parameter_set_value_bool(&parameter_values.data[i], bool_parameter))
            }
            else if (0 == strcmp(get_request->data[i].data, "int_param")) 
            {
              RCSOFTCHECK(rcl_parameter_set_value_integer(&parameter_values.data[i], int_parameter))
            }
            else if (0 == strcmp(get_request->data[i].data, "double_param"))
            {
              RCSOFTCHECK(rcl_parameter_set_value_double(&parameter_values.data[i], double_parameter))
            }
            else if (0 == strcmp(get_request->data[i].data, "string_param")) 
            {
              RCSOFTCHECK(rcl_parameter_set_value_bool(&parameter_values.data[i], string_parameter))
            }
          }

          RCSOFTCHECK(rcl_parameter_service_send_get_response(&parameter_service, &request_header, &parameter_values))
          break;
        }
        case RCL_SET_PARAMETERS:
        {
          rcl_interfaces__msg__Parameter__Sequence * set_request;
          rcl_interfaces__msg__SetParametersResult__Sequence set_result;

          set_request = rcl_parameter_service_take_set_request(&parameter_service, &request_header);
          if (!set_request) {
            printf("Error taking set request\n");
            break;
          }

          rcl_interfaces__msg__SetParametersResult__Sequence__init(&set_result, set_request->size);
          for (size_t i = 0; i < set_request->size; ++i) {
            rcl_interfaces__msg__Parameter * param = &set_request->data[i];

            if ((0 == strcmp(param->name.data, "bool_param")) &&
                (param->value.type == rcl_interfaces__msg__ParameterType__PARAMETER_BOOL)) 
            {
              bool_parameter = param->value.bool_value;
              set_result.data[i].successful = true;
              rosidl_generator_c__String__assign(&set_result.data[i].reason, "success");
            } 
            else if ((0 == strcmp(param->name.data, "int_param")) &&
                (param->value.type == rcl_interfaces__msg__ParameterType__PARAMETER_INTEGER))
            {
              int_parameter = param->value.integer_value;
              set_result.data[i].successful = true;
              rosidl_generator_c__String__assign(&set_result.data[i].reason, "success");
            }
            else if ((0 == strcmp(param->name.data, "double_param")) &&
                (param->value.type == rcl_interfaces__msg__ParameterType__PARAMETER_DOUBLE))
            {
              double_parameter = param->value.double_value;
              set_result.data[i].successful = true;
              rosidl_generator_c__String__assign(&set_result.data[i].reason, "success");
            }
            else if ((0 == strcmp(param->name.data, "string_param")) &&
                (param->value.type == rcl_interfaces__msg__ParameterType__PARAMETER_STRING))
            {
              if (sizeof(string_parameter) >= param->value.string_value.size)
              {
                strcpy(string_parameter, param->value.string_value.data);
                set_result.data[i].successful = true;
                rosidl_generator_c__String__assign(&set_result.data[i].reason, "success");
              }
              else
              {
                set_result.data[i].successful = false;
                rosidl_generator_c__String__assign(&set_result.data[i].reason, "string too large");
              }
            }
            else
            {
              set_result.data[i].successful = false;
              rosidl_generator_c__String__assign(&set_result.data[i].reason, "unknown parameter or invalid type");
            }
          }
          break;
        }
        default:
          break;
      }
    } while (action != RCL_PARAMETER_ACTION_UNKNOWN);
  } while (true);

  /****************************************************************************
   * Finalization.
   ****************************************************************************/
  RCCHECK(rcl_wait_set_fini(&wait_set))
  RCCHECK(rcl_parameter_service_fini(&parameter_service))
  RCCHECK(rcl_node_fini(&node))
  RCCHECK(rcl_shutdown(&context))
  return 0;
}

