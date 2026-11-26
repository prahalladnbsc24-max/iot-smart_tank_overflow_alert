const { SerialPort } = require("serialport");
const { ReadlineParser } = require("@serialport/parser-readline");

const port = new SerialPort({
  path: "COM3",
  baudRate: 9600
});

const parser = port.pipe(new ReadlineParser({ delimiter: "\n" }));

port.on("open", () => console.log("Arduino connected on COM3"));
port.on("error", error => console.error("Serial error:", error.message));

parser.on("data", async line => {
  try {
    const reading = JSON.parse(line.trim());

    if (
      typeof reading.waterLevelPercent !== "number" ||
      typeof reading.waterLevelLitres !== "number" ||
      !["ON", "OFF"].includes(reading.pumpStatus)
    ) {
      console.log("Skipped line with missing or invalid reading fields.");
      return;
    }

    reading.source = "ARDUINO";

    const response = await fetch("http://localhost:5000/api/readings", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(reading)
    });

    if (!response.ok) {
      console.error("Backend rejected reading:", await response.text());
      return;
    }

    console.log("Reading saved:", reading.waterLevelPercent + "%");
  } catch (error) {
    // The Arduino also prints status messages; those aren't JSON readings.
    if (error instanceof SyntaxError) return;
    console.error("Could not send reading:", error.message);
  }
});

process.on("SIGINT", () => {
  port.close(() => process.exit(0));
});