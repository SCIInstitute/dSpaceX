import importlib.util


def run_external_script(script_directory, module_name, method_name, arguments=None):
    """
    Loads and runs an external python script.
    :param script_directory: Where the python script can be found.
    :param module_name: The name of the module to load; this is the name on the python file.
    :param method_name: The name of the method to call.
    :param arguments: The arguments are expected to be a list or None
    :return: The output of the external script.
    """
    spec = importlib.util.spec_from_file_location(module_name, script_directory)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    method_to_call = getattr(module, method_name)
    return method_to_call(arguments)
