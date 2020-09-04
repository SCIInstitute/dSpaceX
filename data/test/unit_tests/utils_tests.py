import unittest

from data.utils import run_external_script


class TestUtils(unittest.TestCase):

    def test_load_script_no_args(self):
        directory = './test_resources/external_script.py'
        module = 'external_script'
        method_name = 'test_sum'

        answer = run_external_script(directory, module, method_name)
        self.assertEqual(answer, 0)

    def test_load_script_with_args(self):
        directory = './test_resources/external_script.py'
        module = 'external_script'
        method_name = 'test_sum'

        answer = run_external_script(directory, module, method_name, arguments=[1, 2, 3])
        self.assertEqual(answer, 6)


if __name__ == '__main__':
    unittest.main()
