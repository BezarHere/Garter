import re

from dataclasses import dataclass

DEFINES_RE = re.compile(r"#\s*define\s+(\w+)(\(.*?\))?\s*(.*?)?\s*?\n")

@dataclass(slots=True, frozen=True)
class Define:
	name: str
	paramters: str | None # for now, we wont parse the paramaters
	value: str | None

def extract_defines(src: str):
	"""currently unable to handle multiline defines"""
	for i in DEFINES_RE.finditer(src):
		yield Define(i[1], i[2], i[3])
