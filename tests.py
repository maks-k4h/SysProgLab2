import subprocess
import re

# build the program
subprocess.run(['make'])

def run_test(sm_path: str, w: str, expected: bool):
    res = subprocess.run(['./main', sm_path, w], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    actual = re.fullmatch(r".*yes.*", res.stdout, re.DOTALL | re.IGNORECASE) is not None
    print(f'\nTESTING {sm_path}, w = {w}, expected: {expected}, actual: {actual}')
    assert expected == actual, res.stdout

# TESTS
# test 0 — one-state s. m. which accepts any word in alphabet
run_test('t0.sm', '-', True)
run_test('t0.sm', 'a', True)
run_test('t0.sm', 'aba', True)
run_test('t0.sm', 'abbbbaa', True)
run_test('t0.sm', 'abbbbac', False) # out of alphabet character

# test 1 — non-deterministic s. m. — must be rejected
run_test('t1.sm', '-', False)
run_test('t1.sm', 'a', False)
run_test('t1.sm', 'ab', False)

# test 2 — s. m. accepts a{bb}a or a{bb}c
run_test('t2.sm', '-', True)
run_test('t2.sm', 'a', True)
run_test('t2.sm', 'c', True)
run_test('t2.sm', 'ab', True)
run_test('t2.sm', 'aba', False) # number of b must be even
run_test('t2.sm', 'abc', False) # number of b must be even
run_test('t2.sm', 'abbbc', False) # number of b must be even
run_test('t2.sm', 'abbbbc', True) 
run_test('t2.sm', 'b', True)
run_test('t2.sm', 'bb', True)
run_test('t2.sm', 'bbbbbbbbbbbb', True)
run_test('t2.sm', 'ac', True)
run_test('t2.sm', 'aac', False)
run_test('t2.sm', 'aacb', False)
run_test('t2.sm', 'aacbccb', False)

# test 3 — s. m. accepts no word
run_test('t3.sm', '-', False)
run_test('t3.sm', 'a', False)
run_test('t3.sm', 'b', False)
run_test('t3.sm', 'ab', False)
run_test('t3.sm', 'bb', False)
run_test('t3.sm', 'ac', False)
run_test('t3.sm', 'aba', False) 
run_test('t3.sm', 'abc', False) 
run_test('t3.sm', 'aac', False)
run_test('t3.sm', 'aacb', False)
run_test('t3.sm', 'aacbccb', False)
run_test('t3.sm', 'bbbbbbbbbbbb', False)

# test 4 — s. m. accepts {z}
run_test('t4.sm', '-', True)
run_test('t4.sm', 'z', True)
run_test('t4.sm', 'a', False)
run_test('t4.sm', 'k', False)
run_test('t4.sm', 'zzzz', True)

# test 5 — s. m. tires to use 27 letters
run_test('t5.sm', '-', False)
run_test('t5.sm', 'z', False)
run_test('t5.sm', 'a', False)
run_test('t5.sm', 'k', False)
run_test('t5.sm', 'zzzz', False)