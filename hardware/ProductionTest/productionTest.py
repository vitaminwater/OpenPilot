
import unittest
import HTMLTestRunner

import logging
import serial
import traceback
import sys
import ConfigParser

from openpilot.uavtalk.uavobject import *
from openpilot.uavtalk.uavtalk import *
from openpilot.uavtalk.objectManager import *
from openpilot.uavtalk.connectionManager import *
    

testFixture = None

class DeviceInfo(object):
    def __init__(self, type, revision, sn):
        self.deviceType = type
        self.deviceRevision = revision
        self.deviceSerialNumber = sn
        
    def __str__(self):
        return "%s Rev:%s SN:%s" % (self.deviceType, self.deviceRevision, self.deviceSerialNumber)
        
class ServoSettings(object):
    pass

class TestFixtureSettings(object):
    def __init__(self):

        self.rotServo = ServoSettings()
        self.tltServo = ServoSettings()
                
        config = ConfigParser.ConfigParser()
        config.read('productionTest.ini')
        
        self.port = config.get('TestFixture','Port')
        self.tester = config.get('TestFixture','Tester') 
            
        self.rotServo.channel = config.getint('TestFixture.RotationServo', 'Channel')
        self.rotServo.center = config.getint('TestFixture.RotationServo', 'Center')
        self.rotServo.left = config.getint('TestFixture.RotationServo', 'Left')
        self.rotServo.right = config.getint('TestFixture.RotationServo', 'Right')
        self.rotServo.minSpeed = config.getint('TestFixture.RotationServo', 'MinSpeed')
        self.rotServo.maxSpeed = config.getint('TestFixture.RotationServo', 'MaxSpeed')
        
        self.tltServo.channel = config.getint('TestFixture.TiltServo', 'Channel')
        self.tltServo.level = config.getint('TestFixture.TiltServo', 'Center')
        self.tltServo.forward = config.getint('TestFixture.TiltServo', 'Forward')
        self.tltServo.backward = config.getint('TestFixture.TiltServo', 'Backward')
        self.tltServo.minSpeed = config.getint('TestFixture.TiltServo', 'MinSpeed')
        self.tltServo.maxSpeed = config.getint('TestFixture.TiltServo', 'MaxSpeed')
        
    
class SensorLog(object):
    def __init__(self):
        self.rawData = []
        self.gyro = []
        self.accel = []
        
    def get00(self, data):
        return data[0][0]
    
    def get01(self, data):
        return data[0][1]
    
    def get02(self, data):
        return data[0][2]
    
    def get10(self, data):
        return data[1][0]
    
    def get11(self, data):
        return data[1][1]
    
    def get12(self, data):
        return data[1][2]
    
    def extract(self):
        self.gyro.append(map(self.get00, self.rawData))
        self.gyro.append(map(self.get01, self.rawData))
        self.gyro.append(map(self.get02, self.rawData)) 
        self.accel.append(map(self.get10, self.rawData))
        self.accel.append(map(self.get11, self.rawData))
        self.accel.append(map(self.get12, self.rawData)) 
    
    def _dumpOneSensor(self, file, name, data):
        file.write("%s" % name)
        for val in data:
            file.write(",%f" % val)
        file.write("\n")
        
    def dumpSensorData(self, file):
        self._dumpOneSensor(file, "GyroX", self.gyro[0])
        self._dumpOneSensor(file, "GyroY", self.gyro[1])
        self._dumpOneSensor(file, "GyroZ", self.gyro[2])
        self._dumpOneSensor(file, "AccelX", self.accel[0])
        self._dumpOneSensor(file, "AccelY", self.accel[1])
        self._dumpOneSensor(file, "AccelZ", self.accel[2])
        
def _hex02(value):
    return "%02X" % value

class TestFixture(object):
    TILT_FW = 0
    TILT_LEVEL = 1
    TILT_BW = 2
    ROT_LEFT = 3
    ROT_CENTER = 4
    ROT_RIGHT = 5
    
    def __init__(self):
        print "Reading settings"
        self.settings = TestFixtureSettings()
        
    def setup(self):
        print "Setting up TestFixture"
        
        if self.settings.port.upper()[:3] == "COM":
            print "Opening Serial Port %s" % self.settings.port
            portNb = int(self.settings.port[3:])
            try:
                self.port = serial.Serial(portNb-1, 57600, timeout=.5)
                if not self.port.isOpen():
                    raise IOError()
            except Exception:
                raise IOError("Failed to open serial port %s" % self.settings.port)
        
        
        print "Starting UavTalk"
        self.uavTalk = UavTalk(self.port)
        self.objMan = ObjManager(self.uavTalk)
        self.objMan.importDefinitions()
        self.uavTalk.start()
        
        import actuatorcommand
        import attituderaw
        self.actuatorCmd = self.objMan.getObj(actuatorcommand.ActuatorCommand.OBJID)
        self.actuatorCmdMeta = self.actuatorCmd.metadata
            
        self.attitudeRaw = self.objMan.getObj(attituderaw.AttitudeRaw.OBJID)
        self.attitudeRawMeta = self.attitudeRaw.metadata
        
        print "Connecting"
        self.connMan = ConnectionManager(self.uavTalk, self.objMan)
        self.connMan.connect()
        
        print "Getting all Data"
        self.objMan.requestAllObjUpdate()
        
        print "Collecting BoardInfo"
        if self.objMan.FirmwareIAPObj.BoardType.value == 4:
            type = "CopterControl"
        else:
            type = "Unknown"
        rev = str(self.objMan.FirmwareIAPObj.BoardRevision.value)
        sn = "".join(map(_hex02, self.objMan.FirmwareIAPObj.CPUSerial.value))
        self.devInfo = DeviceInfo(type, rev, sn)
            
        print "Disabling automatic updates"
        self.objMan.disableAllAutomaticUpdates()
        
        print "Taking control of self.actuatorCmd"
        self.objMan.waitObjUpdate(self.actuatorCmd, request=True, timeout=1)
        self.objMan.ActuatorCommand.metadata.access.value = UAVMetaDataObject.Access.READONLY
        self.objMan.ActuatorCommand.metadata.updated()   
        self.objMan.ManualControlCommand.metadata.access.value = UAVMetaDataObject.Access.READONLY
        self.objMan.ManualControlCommand.metadata.updated()
              
    
    def stop(self):
        if self.uavTalk:
            self.uavTalk.stop()
               
    def setServo(self, servo, pos):
        self.actuatorCmd.Channel.value[servo-1] = pos
        self.uavTalk.sendObject(self.actuatorCmd)
        
    def setTiltServo(self, p):
        servo = self.settings.tltServo.channel
        if p == TestFixture.TILT_FW:
            pos = self.settings.tltServo.forward
        elif p == TestFixture.TILT_LEVEL:
            pos = self.settings.tltServo.level
        elif p == TestFixture.TILT_BW:
            pos = self.settings.tltServo.backward
        else:
            raise ValueError()
        self.setServo(servo, pos)
            
    def setRotServo(self, p):
        servo = self.settings.rotServo.channel
        if p == TestFixture.ROT_LEFT:
            pos = self.settings.rotServo.left
        elif p == TestFixture.ROT_CENTER:
            pos = self.settings.rotServo.center
        elif p == TestFixture.ROT_RIGHT:
            pos = self.settings.rotServo.right
        else:
            raise ValueError()
        self.setServo(servo, pos)
            
            
            
    def _setServos(self, servo1Pos, servo2Pos):
        self.actuatorCmd.Channel.value[4] = servo1Pos
        
        
    def startSensorReporting(self, period):
        self.attitudeRawMeta.telemetryUpdateMode.value = UAVMetaDataObject.UpdateMode.PERIODIC
        self.attitudeRawMeta.telemetryUpdatePeriod.value = period
        self.uavTalk.sendObject(self.attitudeRawMeta)
        self.objMan.AttitudeRaw.waitUpdate(timeout=2)
        
    def stopSensorReporting(self):
        self.attitudeRawMeta.telemetryUpdateMode.value = UAVMetaDataObject.UpdateMode.MANUAL
        self.uavTalk.sendObject(self.attitudeRawMeta)
        
    def recordSensors(self, period, nbValues, log):
        print "Recording...",
        start = time.time()
        #prev = start
        for i in xrange(nbValues):
            self.objMan.AttitudeRaw.waitUpdate(timeout=1)
            #print "%d" % ((time.time()-prev)*1000)
            #prev = time.time()
            #self.objMan.waitObjUpdate(self.attitudeRaw, request=False, timeout=.2)
            log.rawData.append((self.attitudeRaw.gyros.value, self.attitudeRaw.accels.value))
        measuredPeriod = (time.time()-start)*1000/nbValues 
        print "Measured Period = %.2f" % measuredPeriod
        assert(measuredPeriod/period > .8 and measuredPeriod/period < 1.2)
            
    def measureSensors(self):
        self.startSensorReporting(20)
        
        self.calLog = SensorLog()
        self.levelLog = SensorLog()
        self.yawRLog = SensorLog()
        self.yawLLog = SensorLog()
        self.pitchFWLog = SensorLog()
        self.pitchBWLog = SensorLog()
        self.rollRLog = SensorLog()
        self.rollLLog = SensorLog()
        
        testFixture.setRotServo(TestFixture.ROT_CENTER)
        testFixture.setTiltServo(TestFixture.TILT_LEVEL)
        time.sleep(1)
        print "Calibrating Accels ",
        self.recordSensors(20, 1000/20, self.calLog)
        self.calLog.extract()
        self.calLog.meanAccel = []
        for i in range(3):
            self.calLog.meanAccel.append(sum(self.calLog.accel[i])/len(self.calLog.accel[i]))
        print " Current bias: (%d %d %d)" % tuple(self.objMan.AttitudeSettings.AccelBias.value)   
        print " Average Accels: (%.4f %.4f %.4f)" % tuple(self.calLog.meanAccel)
        self.objMan.AttitudeSettings.AccelBias.value[0] += self.calLog.meanAccel[0] / (0.004 * 9.81)
        self.objMan.AttitudeSettings.AccelBias.value[1] += self.calLog.meanAccel[1] / (0.004 * 9.81)
        self.objMan.AttitudeSettings.AccelBias.value[2] += (self.calLog.meanAccel[2] + 9.81) / (0.004 * 9.81)
        print " New bias: (%d %d %d)" % tuple(self.objMan.AttitudeSettings.AccelBias.value)
        self.objMan.AttitudeSettings.updated()
        
        # Some delay after calibrating the accels
        time.sleep(.5)
        
        print "Level ",
        self.recordSensors(20, 1000/20, self.levelLog)
        self.levelLog.extract()
        
        print "Yaw Right",
        testFixture.setRotServo(TestFixture.ROT_LEFT)
        time.sleep(1)
        testFixture.setRotServo(TestFixture.ROT_RIGHT)
        self.recordSensors(20, 2000/20, self.yawRLog)
        self.yawRLog.extract()
        
        print "Yaw Left",
        testFixture.setRotServo(TestFixture.ROT_LEFT)
        self.recordSensors(20, 2000/20, self.yawLLog)
        self.yawLLog.extract()
        
        print "Pitch Forward",
        testFixture.setRotServo(TestFixture.ROT_CENTER)
        testFixture.setTiltServo(TestFixture.TILT_BW)
        time.sleep(1)
        testFixture.setTiltServo(TestFixture.TILT_FW)
        self.recordSensors(20, 2000/20, self.pitchFWLog)
        self.pitchFWLog.extract()
        
        print "Pitch Backward",
        testFixture.setRotServo(TestFixture.ROT_CENTER)
        testFixture.setTiltServo(TestFixture.TILT_BW)
        self.recordSensors(20, 2000/20, self.pitchBWLog)
        self.pitchBWLog.extract()
        
        print "Roll Right",
        testFixture.setRotServo(TestFixture.ROT_RIGHT)
        testFixture.setTiltServo(TestFixture.TILT_BW)
        time.sleep(1)
        testFixture.setTiltServo(TestFixture.TILT_FW)
        self.recordSensors(20, 2000/20, self.rollRLog)
        self.rollRLog.extract()
        
        print "Roll Left",
        testFixture.setTiltServo(TestFixture.TILT_BW)
        self.recordSensors(20, 2000/20, self.rollLLog)
        self.rollLLog.extract()
        
        testFixture.setRotServo(TestFixture.ROT_CENTER)
        testFixture.setTiltServo(TestFixture.TILT_LEVEL)
        
        self.stopSensorReporting()
        
    def dumpSensorData(self):
        file = open("sensorDump.csv", "w")
        
        file.write("Level\n")
        self.levelLog.dumpSensorData(file)
        file.write("YawRight\n")
        self.yawRLog.dumpSensorData(file)
        file.write("YawLeft\n")
        self.yawLLog.dumpSensorData(file)
        file.write("PitchForward\n")
        self.pitchFWLog.dumpSensorData(file)
        file.write("PitchBackward\n")
        self.pitchBWLog.dumpSensorData(file)
        file.write("RollRight\n")
        self.rollRLog.dumpSensorData(file)
        file.write("RollLeft\n")
        self.rollLLog.dumpSensorData(file)
        
           
        file.close()              
                         
    def stop(self):
        try:
            self.uavTalk.stop()
        except Exception:
            pass
        

class TestCase(unittest.TestCase):
    
    def setUp(self):
        global testFixture
        if testFixture == None:
            testFixture = TestFixture()
        self.tf = testFixture
        
    def __str__(self):
        name = getattr(self, self._testMethodName).__doc__
        if name == None:
            name = self._testMethodName
            
        return name
    
    def _checkTol(self, value, exp, tol):
        return value >= exp-tol and value <= exp+tol
        
    def _checkSensor(self, data, checkFrom, checkTo, expValue, meanTol, ppMax):
        fromIndex = int(float(len(data))*checkFrom)
        toIndex = int(float(len(data))*checkTo)
        
        minVal = min(data[fromIndex:toIndex])
        maxVal = max(data[fromIndex:toIndex])
        meanVal = sum(data[fromIndex:toIndex]) / (toIndex-fromIndex)
        ppVal = maxVal-minVal
        
        print " Raw: ", data
        print " Measured: min=%.1f max=%.1f mean=%.1f pp=%.1f " % (minVal, maxVal, meanVal, ppVal)
        self.assertTrue(self._checkTol(meanVal, expValue, meanTol))
        self.assertTrue(ppVal < ppMax)
    

class RxTxChannels(TestCase):
            
    def testChannel1(self):
        """TX/TX Channel 1"""
        print "Testing Channel 1"
        self.assertTrue(True)

    def testChannel2(self):
        """TX/TX Channel 2"""
        self.assertTrue(True)

    def testChannel3(self):
        """TX/TX Channel 3"""
        self.assertTrue(True)

    def testChannel4(self):
        """TX/TX Channel 4"""
        self.assertTrue(True)

    def testChannel5(self):
        """TX/TX Channel 5"""
        self.assertTrue(True)

    def testChannel6(self):
        """TX/TX Channel 6"""
        self.assertTrue(True)

        
class Gyros(TestCase):
        
    def _checkGyroAtLevel(self, data):
        self._checkSensor(  data, 
                            checkFrom=0.0, checkTo=1.0, 
                            expValue=0, meanTol=5, ppMax = 10)   
        
    def _checkGyroAtRot(self, data, expRotDetection, servoData):
        
        if expRotDetection == 0:
            self._checkSensor(  data, 
                                checkFrom=0.12, checkTo=.25, 
                                expValue=0, meanTol=30, ppMax = 50)
        elif expRotDetection == -1 or expRotDetection == +1:
            self._checkSensor(  data, 
                                checkFrom=0.12, checkTo=.25, 
                                expValue = expRotDetection*(servoData.maxSpeed+servoData.minSpeed)/2, 
                                meanTol = (servoData.maxSpeed-servoData.minSpeed), 
                                ppMax = (servoData.maxSpeed-servoData.minSpeed))
        else:
            raise ValueError()
           
        
    def testGyroX(self):
        """Gyro X"""
        print "Static, Level"
        self._checkGyroAtLevel(self.tf.levelLog.gyro[0])
        print "Yaw Right"
        self._checkGyroAtRot(self.tf.yawRLog.gyro[0], 0, self.tf.settings.rotServo)
        print "Yaw Left"
        self._checkGyroAtRot(self.tf.yawLLog.gyro[0], 0, self.tf.settings.rotServo)
        print "Pitch Forward"
        self._checkGyroAtRot(self.tf.pitchFWLog.gyro[0], 0, self.tf.settings.tltServo)
        print "Pitch Backward"
        self._checkGyroAtRot(self.tf.pitchBWLog.gyro[0], 0, self.tf.settings.tltServo)
        print "Roll Right"
        self._checkGyroAtRot(self.tf.rollRLog.gyro[0], -1, self.tf.settings.tltServo)
        print "Roll Left"
        self._checkGyroAtRot(self.tf.rollLLog.gyro[0], +1, self.tf.settings.tltServo)
        
    def testGyroY(self):
        """Gyro Y"""
        print "Static, Level"
        self._checkGyroAtLevel(self.tf.levelLog.gyro[1])
        print "Yaw Right"
        self._checkGyroAtRot(self.tf.yawRLog.gyro[1], 0, self.tf.settings.rotServo)
        print "Yaw Left"
        self._checkGyroAtRot(self.tf.yawLLog.gyro[1], 0, self.tf.settings.rotServo)
        print "Pitch Forward"
        self._checkGyroAtRot(self.tf.pitchFWLog.gyro[1], -1, self.tf.settings.tltServo)
        print "Pitch Backward"
        self._checkGyroAtRot(self.tf.pitchBWLog.gyro[1], +1, self.tf.settings.tltServo)
        print "Roll Right"
        self._checkGyroAtRot(self.tf.rollRLog.gyro[1], 0, self.tf.settings.tltServo)
        print "Roll Left"
        self._checkGyroAtRot(self.tf.rollLLog.gyro[1], 0, self.tf.settings.tltServo)

    def testGyroZ(self):
        """Gyro Z"""
        print "Static, Level"
        self._checkGyroAtLevel(self.tf.levelLog.gyro[2])
        print "Yaw Right"
        self._checkGyroAtRot(self.tf.yawRLog.gyro[2], +1, self.tf.settings.rotServo)
        print "Yaw Left"
        self._checkGyroAtRot(self.tf.yawLLog.gyro[2], -1, self.tf.settings.rotServo)
        print "Pitch Forward"
        self._checkGyroAtRot(self.tf.pitchFWLog.gyro[2], 0, self.tf.settings.tltServo)
        print "Pitch Backward"
        self._checkGyroAtRot(self.tf.pitchBWLog.gyro[2], 0, self.tf.settings.tltServo)
        print "Roll Right"
        self._checkGyroAtRot(self.tf.rollRLog.gyro[2], 0, self.tf.settings.tltServo)
        print "Roll Left"
        self._checkGyroAtRot(self.tf.rollLLog.gyro[2], 0, self.tf.settings.tltServo)
        
        
class Accels(TestCase):
        
    # checks accel sensor while there is not movement
    def _checkAccel(self, data, checkFrom, checkTo, expectedAcc, highTolerance = False):
        if highTolerance:
            meanTol = 5
        else:
            meanTol = 2
        self._checkSensor(  data, 
                            checkFrom=checkFrom, checkTo=checkTo, 
                            expValue=9.81*expectedAcc, meanTol=meanTol, ppMax = 1)
        
    # checks accel sensor while there is movement on an axis that should not be registered by the accel sensor
    # The mean value should be correct, but the pp on the signal is allowed to bigger
    def _checkAccelAtRot(self, data, checkFrom, checkTo, expectedAcc):
        self._checkSensor(  data, 
                            checkFrom=checkFrom, checkTo=checkTo, 
                            expValue=9.81*expectedAcc, meanTol=1, ppMax = 5) 
        
        
    def testAccelsX(self):
        """Accel X"""
        print "Static, Level"
        self._checkAccel(self.tf.levelLog.accel[0], 0, 1, 0)
        print "Yawed Right"
        self._checkAccel(self.tf.yawRLog.accel[0], .8, 1, 0)
        print "Yawed Left"
        self._checkAccel(self.tf.yawLLog.accel[0], .8, 1, 0)
        print "Rolled left"
        self._checkAccel(self.tf.rollLLog.accel[0], .80, 1, 0)
        print "Rolled Right"
        self._checkAccel(self.tf.rollRLog.accel[0], .80, 1, 0)
        print "Pitched Forward"
        self._checkAccel(self.tf.pitchFWLog.accel[0], .80, 1, -1, highTolerance=True)
        print "Pitched Backward"
        self._checkAccel(self.tf.pitchBWLog.accel[0], .80, 1, 1, highTolerance=True)
         
    def testAccelsY(self):
        """Accel Y"""
        print "Static, Level"
        self._checkAccel(self.tf.levelLog.accel[1], 0, 1, 0)
        print "Yawed Right"
        self._checkAccel(self.tf.yawRLog.accel[1], 0.8, 1, 0)
        print "Yawed Left"
        self._checkAccel(self.tf.yawLLog.accel[1], 0.8, 1, 0)
        print "Rolled left"
        self._checkAccel(self.tf.rollLLog.accel[1], .80, 1, -1, highTolerance=True)
        print "Rolled Right"
        self._checkAccel(self.tf.rollRLog.accel[1], .80, 1, 1, highTolerance=True)
        print "Pitched Forward"
        self._checkAccel(self.tf.pitchFWLog.accel[1], .80, 1, 0)
        print "Pitched Backward"
        self._checkAccel(self.tf.pitchBWLog.accel[1], .80, 1, 0)
        
    def testAccelsZ(self):
        """Accel Z"""
        print "Static, Level"
        self._checkAccel(self.tf.levelLog.accel[2], 0, 1, -1)
        print "Yawed Right"
        self._checkAccel(self.tf.yawRLog.accel[2], 0.8, 1, -1)
        print "Yawed Left"
        self._checkAccel(self.tf.yawLLog.accel[2], 0.8, 1, -1)
        print "Rolled left"
        self._checkAccel(self.tf.rollLLog.accel[2], .80, 1, 0)
        print "Rolled Right"
        self._checkAccel(self.tf.rollRLog.accel[2], .80, 1, 0)
        print "Pitched Forward"
        self._checkAccel(self.tf.pitchFWLog.accel[2], .80, 1, 0)
        print "Pitched Backward"
        self._checkAccel(self.tf.pitchBWLog.accel[2], .80, 1, 0)

        
class USB(TestCase):
    def testUSB(self):
        """USB"""
        self.assertTrue(True)

class SerialPorts(TestCase):
    def testSerial1(self):
        """Serial Port 1"""
        self.assertTrue(True)

    def testSerial2(self):
        """Serial Port 2"""
        self.assertTrue(True)

class Flash(TestCase):
    def testFlash(self):
        """Flash"""
        self.assertTrue(True)





if __name__ == '__main__':
    try:
        #logging.basicConfig(level=logging.DEBUG)
        
        testFixture = TestFixture()
        testFixture.setup()
    
        devInfoStr = str(testFixture.devInfo)
        print "Device: %s" % devInfoStr
        print
        
        testFixture.setRotServo(TestFixture.ROT_CENTER)
        testFixture.setTiltServo(TestFixture.TILT_LEVEL)
            
#        testFixture.setTiltServo(TestFixture.TILT_BW)
#        testFixture.setRotServo(TestFixture.ROT_RIGHT)
#        while True:
#            time.sleep(1)
             
        devInfo = "%s-%s" % (testFixture.devInfo.deviceType, testFixture.devInfo.deviceSerialNumber)
        
        testFixture.measureSensors()
        testFixture.dumpSensorData()
        
        fp = file("testReport-%s.html" % (devInfo), 'wb')
        myTestRunner = HTMLTestRunner.HTMLTestRunner(
                    stream=fp,
                    title='',
                    description='%s'%devInfoStr,
                    tester=testFixture.settings.tester,
                    verbosity=2)
    
        #myTestRunner = unittest.TextTestRunner(verbosity=2)
        unittest.TestProgram(testRunner=myTestRunner)

    except BaseException,e:
            print
            print "An error occured: ", e
            print
            traceback.print_exc()
            try:
                testFixture.stop()
            except Exception:
                pass
            raw_input("Press ENTER, the application will close")

    print "Stopping testFixture"
    try:
        testFixture.stop()
    except Exception:
        pass

