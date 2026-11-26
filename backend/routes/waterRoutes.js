const express = require("express");
const router = express.Router();
const WaterReading = require("../models/WaterReading");

// Save a new water tank reading
router.post("/", async (req, res) => {
  try {
    const {
      waterLevelPercent,
      waterLevelLitres,
      waterConsumedLitres,
      energyUsedKwh,
      pumpStatus,
      source
    } = req.body;

    if (
      waterLevelPercent === undefined ||
      waterLevelLitres === undefined ||
      !pumpStatus
    ) {
      return res.status(400).json({
        message: "Required sensor data is missing"
      });
    }

    const reading = await WaterReading.create({
      waterLevelPercent,
      waterLevelLitres,
      waterConsumedLitres,
      energyUsedKwh,
      pumpStatus,
      source: source || "SIMULATED"
    });

    res.status(201).json({
      message: "Reading saved successfully",
      reading
    });
  } catch (error) {
    res.status(500).json({
      message: "Failed to save reading",
      error: error.message
    });
  }
});

// Get the latest reading
router.get("/latest", async (req, res) => {
  try {
    const reading = await WaterReading.findOne()
      .sort({ createdAt: -1 });

    res.json(reading);
  } catch (error) {
    res.status(500).json({
      message: "Failed to retrieve latest reading"
    });
  }
});

// Get historical readings
router.get("/history", async (req, res) => {
  try {
    const readings = await WaterReading.find()
      .sort({ createdAt: -1 })
      .limit(500);

    res.json(readings);
  } catch (error) {
    res.status(500).json({
      message: "Failed to retrieve history"
    });
  }
});

// Get analytics for daily, weekly, or monthly periods
router.get("/analytics", async (req, res) => {
  try {
    const { period } = req.query;
    const allowedPeriods = ["daily", "weekly", "monthly"];

    if (!allowedPeriods.includes(period)) {
      return res.status(400).json({
        message: "Invalid period. Use daily, weekly, or monthly."
      });
    }

    const days = {
      daily: 1,
      weekly: 7,
      monthly: 30
    };

    const startDate = new Date(
      Date.now() - days[period] * 24 * 60 * 60 * 1000
    );

    const groupFormat =
      period === "daily"
        ? "%Y-%m-%d %H:00"
        : "%Y-%m-%d";

    const analytics = await WaterReading.aggregate([
      {
        $match: {
          createdAt: { $gte: startDate }
        }
      },
      {
        $group: {
          _id: {
            $dateToString: {
              format: groupFormat,
              date: "$createdAt",
              timezone: "Asia/Kolkata"
            }
          },
          totalWaterConsumed: {
            $sum: "$waterConsumedLitres"
          },
          totalEnergyUsed: {
            $max: "$energyUsedKwh"
          },
          readingCount: {
            $sum: 1
          }
        }
      },
      {
        $sort: { _id: 1 }
      }
    ]);

    res.json({
      period,
      analytics
    });
  } catch (error) {
    res.status(500).json({
      message: "Failed to retrieve analytics",
      error: error.message
    });
  }
});

module.exports = router;