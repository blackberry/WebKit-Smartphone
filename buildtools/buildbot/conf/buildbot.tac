
from twisted.application import service
from buildbot.slave.bot import BuildSlave

#basedir = r'b:\\'
#basedir = r'c:\\dev\\bb'
basedir = r'c:\\bb'
buildmaster_host = 'md-athens-01.swlab.rim.net'
port = 9989
slavename = 'slave_scottbot'
passwd = 'slave_scottbot'
keepalive = 600
usepty = 0
umask = None
maxdelay = 300
rotateLength = 1000000
maxRotatedFiles = None

application = service.Application('buildslave')
try:
  from twisted.python.logfile import LogFile
  from twisted.python.log import ILogObserver, FileLogObserver
  logfile = LogFile.fromFullPath("twistd.log", rotateLength=rotateLength,
                                 maxRotatedFiles=maxRotatedFiles)
  application.setComponent(ILogObserver, FileLogObserver(logfile).emit)
except ImportError:
  # probably not yet twisted 8.2.0 and beyond, can't set log yet
  pass
s = BuildSlave(buildmaster_host, port, slavename, passwd, basedir,
               keepalive, usepty, umask=umask, maxdelay=maxdelay)
s.setServiceParent(application)

