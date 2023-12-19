from io import StringIO
from itertools import batched
from math import ceil
from typing import Iterable
from preprocessor import extract_defines

def get_wmmsgs(path: str):
	with open(path, 'r') as f:
		defines = extract_defines(f.read())
	for i in defines:
		if i.paramters is None and i.name.startswith('WM_'):
			yield i.name, i.value

def rows(strs: Iterable[str], columns: int = 4, /, *, tabsize: int = 0, padding: int = 1):
	"""for tab sizes > 0, the spaces are replaced with tabs"""
	ss = StringIO()
	clens = [0 for i in range(columns)]

	# columns width
	for i in batched(strs, columns):
		for j, v in enumerate(i):
			clens[j] = max(clens[j], len(v))
	
	for i in range(columns):
		clens[i] += padding
		if tabsize > 0:
			clens[i] = ceil(clens[i] / tabsize)
		

	for i in batched(strs, columns):
		for j, v in enumerate(i):
			
			ss.write(v)
			# ss.write(',')
			if tabsize > 0:
				ss.write('\t' * (clens[j] - ceil(len(v) / tabsize)))
			else:
				ss.write(' ' * (clens[j] - len(v)))
		ss.write('\n')

	return ss.getvalue()

def is_hex(p: str):
	p = p.lower()
	if p[0:2] != '0x':
		return False
	for i in p[2:]:
		if not i in '1234567890abcdef':
			return False
	return True

def main():
	msgs_raw = tuple(get_wmmsgs('winmsgs.txt'))
	msgs: dict[str, int] = {i[3:].lower(): int(v, base=0) for i, v in msgs_raw if is_hex(v)}
	msgs_values = tuple(msgs.values())
	msgs_rows = rows(msgs.keys(), 4, padding=2)

	with open('windows messages.txt', 'w') as f:
		f.write(msgs_rows)

	while True:
		p = input('> ').lower()
		
		if p == 'all':
			print(msgs_rows)
		elif p == 'q':
			quit(0)
		elif is_hex(p) or p.isalnum():
			
			try:
				p = int(p, base=0)
			except Exception as e:
				print(e)

			if p is int:
				f = msgs_values.index(p)
				if f == -1:
					print(f"NO Message with value {hex(p)} ({p})")
				else:
					p = msgs.keys()[f]

		if p in msgs:
			try:
				print(hex(msgs[p]), msgs[p])
			except Exception as e:
				print(e)
		
		

if __name__ == "__main__":
	main()
