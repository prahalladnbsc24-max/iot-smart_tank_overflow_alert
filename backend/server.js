
const express = require("express");
const cors = require("cors");
const mongoose = require("mongoose");
require("dotenv").config();

const waterRoutes = require("./routes/waterRoutes");

const app = express();

app.use(cors());

app.use(express.json());
app.use("/api/readings", waterRoutes);

// Test route
app.get("/", (req, res) => {
  res.json({
    message: "Smart Water Tank Backend is running!"
  });
});

// Connect to MongoDB and start server
const PORT = process.env.PORT || 5000;

mongoose
  .connect(process.env.MONGO_URI)
  .then(() => {
    console.log("MongoDB connected successfully!");

    app.listen(PORT, () => {
      console.log(`Server running on http://localhost:${PORT}`);
    });
  })
  .catch((error) => {
    console.error("MongoDB connection failed:", error.message);
  });
