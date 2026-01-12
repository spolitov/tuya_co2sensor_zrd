import {Zcl} from 'zigbee-herdsman';
import * as m from 'zigbee-herdsman-converters/lib/modernExtend';
import * as exposes from "zigbee-herdsman-converters/lib/exposes";
import * as utils from "zigbee-herdsman-converters/lib/utils";
import * as reporting from "zigbee-herdsman-converters/lib/reporting";

const e = exposes.presets;
const ea = exposes.access;

const calibrateExtend = {
  calibrate: () => {
    const exposes = [
      e
        .enum("calibrate", ea.SET, ["calibrate"])
        .withDescription("Calibrate CO2 sensor")
    ]
    const toZigbee = [
      {
        key: ["calibrate"],
        convertSet: async (entity, key, value, meta) => {
          const payload = {};
          payload.now = Date.now() / 1000;
          await entity.command("msCO2", 0x80, payload, utils.getOptions(meta.mapped, entity))
        }
      }
    ]
    return {
      exposes,
      fromZigbee: [],
      toZigbee,
      isModernExtend: true
    }
  }
};

async function doConfigure(device, coordinatorEndpoint, definition) {
  const endpoint1 = device.getEndpoint(1);
  await reporting.bind(endpoint1, coordinatorEndpoint, ['msCO2', 'msTemperatureMeasurement', 'msRelativeHumidity']);    
  const payload_co2 = [
    {attribute: {ID: 0x4901, type: Zcl.DataType.UTC}, minimumReportInterval: 0, maximumReportInterval: 3000, reportableChange: 0},
  ];
  await endpoint1.configureReporting('msCO2', payload_co2);
};

export default {
    zigbeeModel: ['Mahtan_CO2_DIY'],
    model: 'Mahtan_CO2_DIY',
    vendor: 'Mahtan-DIY',
    description: 'Tuya CO2 sensor patched by Mahtan',
    configure: doConfigure,
    extend: [
        m.deviceAddCustomCluster("msCO2", {
          ID: 0x040D,
          attributes: {
            lastCalibrationTime: {ID: 0x4901, type: Zcl.DataType.UTC},
            calibrationValue: {ID: 0x4900, type: Zcl.DataType.UINT16, write: true},
          },
          commands: {
              calibrate: {
                  ID: 0x80,
                  parameters: [{name: "now", type: Zcl.DataType.UTC}],
              },
          },
          commandsResponse: {},
        }),
        m.numeric({
          name: "co2",
          cluster: "msCO2",
          label: "CO2",
          attribute: "measuredValue",
          reporting: {min: 10, max: 300, change: 50},
          description: "Measured value",
          unit: "ppm",
          access: "STATE_GET",
        }),
        m.numeric({
          name: "temperature",
          cluster: "msTemperatureMeasurement",
          attribute: "measuredValue",
          reporting: {min: 10, max: 300, change: 50},
          description: "Measured temperature value",
          unit: "°C",
          scale: 100,
          access: "STATE_GET",
        }),
        m.numeric({
            name: "humidity",
            cluster: "msRelativeHumidity",
            attribute: "measuredValue",
            description: "Measured relative humidity",
            reporting: {min: 10, max: 300, change: 100},
            unit: "%",
            scale: 100,
            access: "STATE_GET",
        }),
        m.numeric({
          name: "calibrationValue",
          cluster: "msCO2",
          description: "CO2 calibration value",
          attribute: "calibrationValue",
          unit: "ppm",
          valueMin: 400,
          valueMax: 1500,
        }),
        m.numeric({
          name: "lastCalibrationTime",
          cluster: "msCO2",
          description: "Last CO2 calibration time",
          reporting: {min: 1, max: 3000, change: 0},
          attribute: "lastCalibrationTime",
          access: "STATE_GET",
        }),
        calibrateExtend.calibrate()],
};
