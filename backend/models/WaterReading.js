
const mongoose = require("mongoose");

const waterReadingSchema = new mongoose.Schema(
  {
    waterLevelPercent: {
      type: Number,
      required: true,
      min: 0,
      max: 100
    },

    waterLevelLitres: {
      type: Number,
      required: true,
      min: 0
    },

    waterConsumedLitres: {
      type: Number,
      default: 0,
      min: 0
    },

    energyUsedKwh: {
      type: Number,
      default: 0,
      min: 0
    },

    pumpStatus: {
      type: String,
      enum: ["ON", "OFF"],
      required: true
    },

    source: {
      type: String,
      enum: ["SIMULATED", "ARDUINO"],
      default: "SIMULATED"
    }
  },
  {
    timestamps: true
  }
);

module.exports = mongoose.model(
  "WaterReading",
  waterReadingSchema
);
