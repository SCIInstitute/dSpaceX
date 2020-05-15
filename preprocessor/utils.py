import importlib.util


def run_external_script(script_directory, module_name, method_name, arguments=None):
    spec = importlib.util.spec_from_file_location(module_name, script_directory)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    method_to_call = getattr(module, method_name)
    return method_to_call(arguments)
