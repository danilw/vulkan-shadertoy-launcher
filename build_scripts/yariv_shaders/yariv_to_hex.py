import struct
import os
import sys
import subprocess


if len(sys.argv) != 2:
  print('Usage: python %s filename \n output is *.spv *.yariv and *.hex file \n' % sys.argv[0])
  quit()

inputfilepath = sys.argv[1]
outputname = os.path.basename(inputfilepath)
outdir = os.path.dirname(inputfilepath)
ginfile = os.path.basename(inputfilepath)
ooutdir = os.path.join(outdir,"bin");

spirvcompiler = 'glslangValidator'
if os.name == 'nt':
  spirvcompiler += ".exe"

yariv_pack = './yariv_pack'
if os.name == 'nt':
  spirvcompiler += ".exe"

if not os.path.isdir(ooutdir):
  os.mkdir(ooutdir, 0755 );

subprocess.call([spirvcompiler,'-V100',inputfilepath,'-o',os.path.join(ooutdir,ginfile) + '.spv'])

subprocess.call([yariv_pack,os.path.join(ooutdir,ginfile) + '.spv'])

infile = open(os.path.join(ooutdir,ginfile) + '.yariv', 'rb')
outfilepath = os.path.join(ooutdir,outputname + '.hex')
outfile = open(outfilepath, 'w')


lineno = 1
while 1 :
  b = infile.read(1)
  if len(b) == 0 :
    break
  d, = struct.unpack('B', b)
  outfile.write(hex(d) + ',')
  if lineno % 20 == 0:
    outfile.write('\n')
  lineno = lineno + 1
