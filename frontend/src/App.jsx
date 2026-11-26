import { useEffect, useState } from "react";
import {
  AreaChart,
  Area,
  BarChart,
  Bar,
  XAxis,
  YAxis,
  CartesianGrid,
  Tooltip,
  ResponsiveContainer
} from "recharts";
import "./App.css";

function App() {
  const [period, setPeriod] = useState("Weekly");
  const [latest, setLatest] = useState(null);
  const [history, setHistory] = useState([]);
  const [analytics, setAnalytics] = useState([]);
  const [error, setError] = useState("");

  // FETCH LATEST READING, HISTORY AND ANALYTICS
  useEffect(() => {
    const fetchReadings = async () => {
      try {
        const [latestResponse, historyResponse, analyticsResponse] =
          await Promise.all([
            fetch("http://localhost:5000/api/readings/latest"),
            fetch("http://localhost:5000/api/readings/history"),
            fetch(
              `http://localhost:5000/api/readings/analytics?period=${period.toLowerCase()}`
            )
          ]);

        if (
          !latestResponse.ok ||
          !historyResponse.ok ||
          !analyticsResponse.ok
        ) {
          throw new Error("Could not retrieve readings");
        }

        const latestData = await latestResponse.json();
        const historyData = await historyResponse.json();
        const analyticsData = await analyticsResponse.json();

        setLatest(latestData);
        setHistory(historyData);
        setAnalytics(analyticsData.analytics || []);
        setError("");
      } catch (err) {
        setError("Unable to connect to the backend.");
      }
    };

    fetchReadings();

    const interval = setInterval(fetchReadings, 5000);

    return () => clearInterval(interval);
  }, [period]);

  // NUMBER FORMATTING
  const formatNumber = (value) =>
    Number(value || 0).toLocaleString("en-IN", {
      maximumFractionDigits: 2
    });

  // Energy values are very small, so show more decimal places.
  const formatEnergy = (value) =>
    Number(value || 0).toLocaleString("en-IN", {
      maximumFractionDigits: 8
    });

  const waterLevel = latest
    ? Math.min(100, Math.max(0, Number(latest.waterLevelPercent) || 0))
    : 0;

  // CHART LABEL FORMATTING
  const formatChartLabel = (value) => {
    if (period === "Daily") {
      const date = new Date(value.replace(" ", "T") + ":00");

      return date.toLocaleTimeString("en-IN", {
        hour: "2-digit",
        minute: "2-digit",
        hour12: true
      });
    }

    const date = new Date(`${value}T00:00:00`);

    return date.toLocaleDateString("en-IN", {
      day: "2-digit",
      month: "short"
    });
  };

  // WATER ANALYTICS DATA FROM BACKEND
  const waterData = analytics.map((item) => ({
    name: formatChartLabel(item._id),
    usage: Number(item.totalWaterConsumed || 0)
  }));

  // ENERGY ANALYTICS DATA FROM BACKEND
  const energyData = analytics.map((item) => ({
    name: formatChartLabel(item._id),
    energy: Number(item.totalEnergyUsed || 0)
  }));

  // RECENT READINGS
  const recentReadings = history.slice(0, 4);

  return (
    <div className="dashboard">
      {/* SIDEBAR */}
      <aside className="sidebar">
        <div className="brand">
          <div className="brand-icon">💧</div>
          <div>
            <h2>SmartTank</h2>
            <p className="sidebar-subtitle">SMART WATER SYSTEM</p>
          </div>
        </div>

        <p className="nav-heading">MENU</p>

        <nav>
          <a className="active" href="#dashboard">
            <span className="nav-icon">▦</span>
            Dashboard
          </a>
          <a href="#water">
            <span className="nav-icon">◉</span>
            Water Analytics
          </a>
          <a href="#energy">
            <span className="nav-icon">ϟ</span>
            Energy Analytics
          </a>
          <a href="#alerts">
            <span className="nav-icon">♧</span>
            System Alerts
          </a>
        </nav>

        <div className="sidebar-device">
          <div className="device-icon">⌁</div>
          <div>
            <strong>Smart Tank</strong>
            <span>Monitoring device</span>
          </div>
          <span className="device-indicator"></span>
        </div>

        <div className="sidebar-footer">
          <span>SMARTTANK</span>
          <p>IoT Water Management</p>
        </div>
      </aside>

      {/* MAIN CONTENT */}
      <main className="main-content" id="dashboard">
        {/* HEADER */}
        <header className="topbar">
          <div>
            <p className="breadcrumb">OVERVIEW / DASHBOARD</p>
            <h1>Dashboard</h1>
            <p className="header-description">
              Monitor and manage your water tank in real time.
            </p>
          </div>

          <div className="header-right">
            <div className="header-date">
              <span>◷</span>
              {new Date().toLocaleDateString("en-IN", {
                day: "2-digit",
                month: "short",
                year: "numeric"
              })}
            </div>

            <span className={`demo-badge ${latest ? "connected" : ""}`}>
              <span className="badge-dot"></span>
              {latest?.source === "SIMULATED"
                ? "SIMULATED DATA"
                : latest
                  ? "DEVICE DATA"
                  : "WAITING FOR DATA"}
            </span>
          </div>
        </header>

        {error && <p className="error-message">{error}</p>}

        {/* HERO SECTION */}
        <section className="welcome">
          <div className="welcome-content">
            <span className="hero-label">SMART WATER MONITORING</span>
            <h2>
              Water management,
              <br />
              made smarter.
            </h2>
            <p>
              Keep track of your tank levels, water consumption
              and energy usage from one place.
            </p>

            <div className="hero-connection">
              <span className="status-dot"></span>
              {latest
                ? "Backend connection active"
                : "Waiting for sensor reading"}
            </div>
          </div>

          <div className="tank-visual">
            <div className="tank-heading">CURRENT TANK LEVEL</div>

            <div className="tank-container">
              <div
                className="tank-water"
                style={{ height: `${waterLevel}%` }}
              >
                <div className="water-wave"></div>
              </div>

              <div className="tank-level-text">
                <strong>{latest ? `${waterLevel}%` : "--"}</strong>
                <span>CAPACITY</span>
              </div>
            </div>

            <div className="tank-capacity">
              {latest
                ? `${formatNumber(latest.waterLevelLitres)} Litres`
                : "No sensor reading"}
            </div>
          </div>
        </section>

        {/* STATISTICS */}
        <div className="section-heading">
          <div>
            <span className="section-kicker">LIVE MONITORING</span>
            <h2>Tank Overview</h2>
          </div>

          <span className="live-label">
            <span className="status-dot"></span>
            Latest saved reading
          </span>
        </div>

        <section className="stats-grid">
          <div className="stat-card water-stat">
            <div className="stat-top">
              <div className="stat-icon">💧</div>
              <span className="stat-tag">WATER</span>
            </div>
            <p>Current Water Level</p>
            <h2>{latest ? `${waterLevel}%` : "--"}</h2>
            <span>
              {latest
                ? `${formatNumber(latest.waterLevelLitres)} Litres in tank`
                : "Awaiting reading"}
            </span>
          </div>

          <div className="stat-card usage-stat">
            <div className="stat-top">
              <div className="stat-icon">◉</div>
              <span className="stat-tag">CONSUMPTION</span>
            </div>
            <p>Water Consumed</p>
            <h2>
              {latest
                ? `${formatNumber(latest.waterConsumedLitres)}`
                : "--"}
              <small>{latest ? " L" : ""}</small>
            </h2>
            <span>Latest saved reading</span>
          </div>

          <div className="stat-card energy-stat">
            <div className="stat-top">
              <div className="stat-icon">ϟ</div>
              <span className="stat-tag">ENERGY</span>
            </div>
            <p>Energy Used</p>
            <h2>
              {latest
                ? formatEnergy(latest.energyUsedKwh)
                : "--"}
              <small>{latest ? " kWh" : ""}</small>
            </h2>
            <span>Cumulative estimate since Arduino reset</span>
          </div>

          <div className="stat-card pump-stat">
            <div className="stat-top">
              <div className="stat-icon">⚙</div>
              <span className="stat-tag">MOTOR</span>
            </div>
            <p>Pump Status</p>
            <h2 className="pump-value">
              {latest?.pumpStatus || "--"}
            </h2>
            <span>Latest saved reading</span>
          </div>
        </section>

        {/* ANALYTICS */}
        <section className="analytics-section">
          <div className="section-heading">
            <div>
              <span className="section-kicker">PERFORMANCE</span>
              <h2>Consumption Analytics</h2>
            </div>

            <div className="period-buttons">
              {["Daily", "Weekly", "Monthly"].map((item) => (
                <button
                  key={item}
                  className={period === item ? "selected" : ""}
                  onClick={() => setPeriod(item)}
                >
                  {item}
                </button>
              ))}
            </div>
          </div>

          <section className="charts-grid">
            {/* WATER CHART */}
            <div className="chart-card" id="water">
              <div className="chart-title-row">
                <div>
                  <h3>Water Consumption</h3>
                  <p>{period} recorded water usage</p>
                </div>
                <div className="chart-symbol water-symbol">💧</div>
              </div>

              <ResponsiveContainer width="100%" height={260}>
                <AreaChart data={waterData}>
                  <defs>
                    <linearGradient
                      id="waterGradient"
                      x1="0"
                      y1="0"
                      x2="0"
                      y2="1"
                    >
                      <stop
                        offset="0%"
                        stopColor="#20c9e8"
                        stopOpacity={0.4}
                      />
                      <stop
                        offset="95%"
                        stopColor="#20c9e8"
                        stopOpacity={0.02}
                      />
                    </linearGradient>
                  </defs>

                  <CartesianGrid
                    stroke="#263d52"
                    strokeDasharray="4 4"
                  />

                  <XAxis
                    dataKey="name"
                    stroke="#7890a8"
                    tick={{ fontSize: 11 }}
                  />

                  <YAxis
                    stroke="#7890a8"
                    tick={{ fontSize: 11 }}
                  />

                  <Tooltip
                    contentStyle={{
                      background: "#14283d",
                      border: "1px solid #2a4963",
                      borderRadius: "10px",
                      color: "#ffffff"
                    }}
                  />

                  <Area
                    type="monotone"
                    dataKey="usage"
                    name="Water used (L)"
                    stroke="#20c9e8"
                    strokeWidth={3}
                    fill="url(#waterGradient)"
                    activeDot={{ r: 6 }}
                  />
                </AreaChart>
              </ResponsiveContainer>

              {waterData.length === 0 && (
                <p className="empty-message">
                  No water readings for this period.
                </p>
              )}
            </div>

            {/* ENERGY CHART */}
            <div className="chart-card" id="energy">
              <div className="chart-title-row">
                <div>
                  <h3>Cumulative Energy</h3>
                  <p>
                    Highest cumulative reading in each {period.toLowerCase()}{" "}
                    group
                  </p>
                </div>
                <div className="chart-symbol energy-symbol">ϟ</div>
              </div>

              <ResponsiveContainer width="100%" height={260}>
                <BarChart data={energyData}>
                  <CartesianGrid
                    stroke="#263d52"
                    strokeDasharray="4 4"
                  />

                  <XAxis
                    dataKey="name"
                    stroke="#7890a8"
                    tick={{ fontSize: 11 }}
                  />

                  <YAxis
                    stroke="#7890a8"
                    tick={{ fontSize: 11 }}
                    tickFormatter={formatEnergy}
                  />

                  <Tooltip
                    formatter={(value) => [
                      `${formatEnergy(value)} kWh`,
                      "Energy"
                    ]}
                    contentStyle={{
                      background: "#14283d",
                      border: "1px solid #2a4963",
                      borderRadius: "10px",
                      color: "#ffffff"
                    }}
                  />

                  <Bar
                    dataKey="energy"
                    name="Energy (kWh)"
                    fill="#42d6a4"
                    radius={[6, 6, 0, 0]}
                  />
                </BarChart>
              </ResponsiveContainer>

              {energyData.length === 0 && (
                <p className="empty-message">
                  No energy readings for this period.
                </p>
              )}
            </div>
          </section>
        </section>

        {/* BOTTOM PANELS */}
        <section className="bottom-grid">
          {/* RECENT READINGS */}
          <div className="recent-card">
            <div className="panel-heading">
              <div>
                <span className="section-kicker">ACTIVITY</span>
                <h3>Recent Readings</h3>
              </div>
              <span className="record-count">
                {history.length} records
              </span>
            </div>

            {recentReadings.length > 0 ? (
              <div className="recent-list">
                {recentReadings.map((reading) => (
                  <div
                    className="recent-item"
                    key={reading._id || reading.createdAt}
                  >
                    <div className="recent-icon">💧</div>

                    <div className="recent-details">
                      <strong>
                        {formatNumber(reading.waterLevelLitres)} Litres
                      </strong>
                      <span>
                        {new Date(reading.createdAt).toLocaleString("en-IN")}
                      </span>
                    </div>

                    <span className="recent-percent">
                      {reading.waterLevelPercent}%
                    </span>
                  </div>
                ))}
              </div>
            ) : (
              <p className="empty-message">
                No readings have been recorded yet.
              </p>
            )}
          </div>

          {/* SYSTEM STATUS */}
          <div className="alerts-card" id="alerts">
            <div className="panel-heading">
              <div>
                <span className="section-kicker">SYSTEM</span>
                <h3>System Status</h3>
              </div>
              <div className="system-icon">⌁</div>
            </div>

            <div className="system-row">
              <span>Backend Connection</span>
              <strong className={latest ? "good-status" : "waiting-status"}>
                {latest ? "Connected" : "Waiting"}
              </strong>
            </div>

            <div className="system-row">
              <span>Water Pump</span>
              <strong>{latest?.pumpStatus || "Unknown"}</strong>
            </div>

            <div className="system-row">
              <span>Reading Source</span>
              <strong>{latest?.source || "Unknown"}</strong>
            </div>

            <div className="system-note">
              {latest
                ? "Your latest saved tank reading is available."
                : "Waiting for the first tank reading."}
            </div>
          </div>
        </section>

        <footer>
          <span>💧 SmartTank</span>
          <span>IoT Water Monitoring System</span>
        </footer>
      </main>
    </div>
  );
}

export default App;