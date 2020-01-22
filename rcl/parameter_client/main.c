#include <rcl/rcl.h>
#include <rcl/parameter_client.h>
#include <rcl/error_handling.h>

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){printf("Failed status on line %d: %d. Aborting.\n",__LINE__,(int)temp_rc); return 1;}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){printf("Failed status on line %d: %d. Continuing.\n",__LINE__,(int)temp_rc);}}

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

  rcl_parameter_client_t parameter_client = rcl_get_zero_initialized_parameter_client();
  const rcl_parameter_client_options_t pc_options = rcl_parameter_client_get_default_options();
  RCCHECK(rcl_parameter_client_init(&parameter_client, &node, &pc_options))

  rcl_wait_set_t wait_set = rcl_get_zero_initialized_wait_set();
  RCCHECK(rcl_wait_set_init(&wait_set, 1, 0, 0, RCL_NUMBER_OF_PARAMETER_ACTIONS, 0, 0, &context, rcl_get_default_allocator()))

  int64_t seq_num;
  rcl_param_action_t action;
  rmw_request_id_t request_header;
  const size_t num_params = 4;

  /****************************************************************************
   * Get parameter list.
   ****************************************************************************/
  rcl_interfaces__msg__ListParametersResult parameter_list;
  rosidl_generator_c__String__Sequence prefixes;
  rcl_interfaces__msg__ListParametersResult * list_parameters_result;

  rcl_interfaces__msg__ListParametersResult__init(&parameter_list);
  rosidl_generator_c__String__Sequence__init(&prefixes, 0);
  RCCHECK(rcl_parameter_client_send_list_request(&parameter_client, &prefixes, 0, &seq_num))

  RCCHECK(rcl_wait_set_clear(&wait_set))
  RCCHECK(rcl_wait_set_add_parameter_client(&wait_set, &parameter_client))
  RCCHECK(rcl_wait(&wait_set, RCL_MS_TO_NS(50)))

  RCCHECK(rcl_parameter_client_get_pending_action(&wait_set, &parameter_client, &action))
  if (action != RCL_LIST_PARAMETERS) {
    printf("Error when RCL_LIST_PARAMETERS is expected\n");
    return 1;
  }
  list_parameters_result = rcl_parameter_client_take_list_response(&parameter_client, &request_header);
  if ((NULL == list_parameters_result) || (request_header.sequence_number != seq_num)) {
    printf("Error taking parameter list\n");
    return 1;
  } else {
    printf("Parameters list: ");
    for (size_t i = 0; i < list_parameters_result->names.size; ++i) {
      printf("%s ", list_parameters_result->names.data[i].data);
    } 
    printf("\n");
  }

  /****************************************************************************
   * Set parameters.
   ****************************************************************************/
  rcl_interfaces__msg__Parameter__Sequence parameters;
  rcl_interfaces__msg__SetParametersResult__Sequence * set_parameters_result;

  rcl_interfaces__msg__Parameter__Sequence__init(&parameters, num_params);
  RCCHECK(rcl_parameter_set_bool(&parameters.data[0], "bool_param", true))
  RCCHECK(rcl_parameter_set_integer(&parameters.data[1], "int_param", 11))
  RCCHECK(rcl_parameter_set_double(&parameters.data[2], "double_param", 89.0))
  RCCHECK(rcl_parameter_set_string(&parameters.data[3], "string_param", "hello woald"))

  RCCHECK(rcl_parameter_client_send_set_request(&parameter_client, &parameters, &seq_num))
  RCCHECK(rcl_wait_set_clear(&wait_set))
  RCCHECK(rcl_wait_set_add_parameter_client(&wait_set, &parameter_client))
  RCCHECK(rcl_wait(&wait_set, RCL_MS_TO_NS(50)))

  RCCHECK(rcl_parameter_client_get_pending_action(&wait_set, &parameter_client, &action))
  if (action != RCL_SET_PARAMETERS) {
    printf("Error when RCL_SET_PARAMETERS is expected\n");
    return 1;
  }
  set_parameters_result = rcl_parameter_client_take_set_response(&parameter_client, &request_header);
  if ((NULL == set_parameters_result) || 
      (request_header.sequence_number != seq_num) || 
      (set_parameters_result->size != num_params)) {
    printf("Error taking set parameter result\n");
    return 1;
  } else {
    printf("Parameters set result: ");
    for (size_t i = 0; i < num_params; ++i) {
      printf("%s ", set_parameters_result->data[i].reason.data);
    }
    printf("\n");
  }

  /****************************************************************************
   * Get parameters.
   ****************************************************************************/
  rosidl_generator_c__String__Sequence parameter_names;
  rcl_interfaces__msg__ParameterValue__Sequence * parameter_values;

  rosidl_generator_c__String__Sequence__init(&parameter_names, num_params);
  rcl_interfaces__msg__ParameterValue__Sequence__init(parameter_values, num_params);
  rosidl_generator_c__String__assign(&parameter_names.data[0], "bool_param");
  rosidl_generator_c__String__assign(&parameter_names.data[1], "int_param");
  rosidl_generator_c__String__assign(&parameter_names.data[2], "double_param");
  rosidl_generator_c__String__assign(&parameter_names.data[3], "string_param");

  RCCHECK(rcl_parameter_client_send_get_request(&parameter_client, &parameter_names, &seq_num))
  RCCHECK(rcl_wait_set_clear(&wait_set))
  RCCHECK(rcl_wait_set_add_parameter_client(&wait_set, &parameter_client))
  RCCHECK(rcl_wait(&wait_set, RCL_MS_TO_NS(50)))

  RCCHECK(rcl_parameter_client_get_pending_action(&wait_set, &parameter_client, &action))
  if (action != RCL_GET_PARAMETERS) {
    printf("Error when RCL_GET_PARAMETERS is expected\n");
    return 1;
  }
  parameter_values = rcl_parameter_client_take_get_response(&parameter_client, &request_header);
  if ((NULL == parameter_values) || 
      (request_header.sequence_number != seq_num) || 
      (parameter_values->size != num_params)) {
    printf("Error taking parameter value\n");
    return 1;
  } else {
    printf("Parameters values: ");
    for (size_t i = 0; i < num_params; ++i) {
      switch (parameter_values->data[i].type)
      {
        case rcl_interfaces__msg__ParameterType__PARAMETER_BOOL:
        {
          printf("(bool)%d ", parameter_values->data[i].bool_value);
          break;
        }
        case rcl_interfaces__msg__ParameterType__PARAMETER_INTEGER:
        {
          printf("(int)%ld ", parameter_values->data[i].integer_value);
          break;
        }
        case rcl_interfaces__msg__ParameterType__PARAMETER_DOUBLE:
        {
          printf("(double)%lf ", parameter_values->data[i].double_value);
          break;
        }
        case rcl_interfaces__msg__ParameterType__PARAMETER_STRING:
        {
          printf("(string)%s ", parameter_values->data[i].string_value.data);
          break;
        }
        default:
          break;
      }
      printf("%s ", set_parameters_result->data[i].reason.data);
    }
    printf("\n");
  }

  /****************************************************************************
   * Finalization.
   ****************************************************************************/
  RCCHECK(rcl_wait_set_fini(&wait_set))
  RCCHECK(rcl_parameter_client_fini(&parameter_client))
  RCCHECK(rcl_node_fini(&node))
  RCCHECK(rcl_shutdown(&context))
  return 0;
}

