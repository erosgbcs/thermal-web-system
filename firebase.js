      import { initializeApp } from "https://www.gstatic.com/firebasejs/10.8.0/firebase-app.js";
      import {
        getDatabase,
        ref,
        onValue,
        set,
      } from "https://www.gstatic.com/firebasejs/10.8.0/firebase-database.js";
      import {
        getAuth,
        signInWithEmailAndPassword,
        signOut,
        onAuthStateChanged,
        sendPasswordResetEmail,
      } from "https://www.gstatic.com/firebasejs/10.8.0/firebase-auth.js";

      const firebaseConfig = {
        apiKey: "AIzaSyCuq5d5FKDlahLEkzVCHLqrScNhNHl2iIs",
        authDomain: "web-app-thermal-detection.firebaseapp.com",
        databaseURL:
          "https://web-app-thermal-detection-default-rtdb.firebaseio.com",
        projectId: "web-app-thermal-detection",
        storageBucket: "web-app-thermal-detection.firebasestorage.app",
        messagingSenderId: "231355651791",
        appId: "1:231355651791:web:d394151424a257ddefc9ba",
        measurementId: "G-LYHV1QBRSR",
      };

      const app = initializeApp(firebaseConfig);
      const database = getDatabase(app);
      const auth = getAuth(app);

      $forgotPasswordBtn.addEventListener("click", async () => {
        const email = $authEmail.value.trim();
        $authError.innerText = "";
        $authSuccess.innerText = "";

        if (!email) {
          $authError.innerText = "Please enter your email address first.";
          return;
        }

        try {
          await sendPasswordResetEmail(auth, email);
          $authSuccess.innerText =
            "Password reset email sent. Check your inbox.";
        } catch (error) {
          console.error("Forgot password error:", error);
          let msg = error.message || "Unable to send password reset email.";
          if (error.code === "auth/user-not-found")
            msg = "No account found with this email.";
          if (error.code === "auth/invalid-email")
            msg = "Invalid email address format.";
          $authError.innerText = msg;
        }
      });

      $authSubmitBtn.addEventListener("click", async () => {
        const email = $authEmail.value.trim();
        const password = $authPassword.value.trim();

        $authError.innerText = "";
        $authSuccess.innerText = "";

        if (!email || !password) {
          $authError.innerText = "Please enter both admin email and password.";
          return;
        }
        if (password.length < 6) {
          $authError.innerText =
            "Admin password must be at least 6 characters.";
          return;
        }

        $authSubmitBtn.disabled = true;
        $authSubmitBtn.innerText = "Please wait...";

        try {
          await signInWithEmailAndPassword(auth, email, password);
        } catch (error) {
          console.error("Auth error:", error);
          let msg = error.message || "Admin authentication failed.";
          if (error.code === "auth/user-not-found")
            msg = "No admin account found with this email.";
          if (error.code === "auth/wrong-password")
            msg = "Incorrect admin password. Please try again.";
          if (error.code === "auth/invalid-credential")
            msg = "Incorrect admin email or password. Please try again.";
          if (error.code === "auth/invalid-email")
            msg = "Invalid admin email address format.";
          if (error.code === "auth/operation-not-allowed")
            msg = "Email/password sign-in is not enabled in Firebase.";
          $authError.innerText = msg;
        } finally {
          $authSubmitBtn.disabled = false;
          $authSubmitBtn.innerText = "Sign In";
        }
      });

      document.addEventListener("keydown", (e) => {
        if (
          e.key === "Enter" &&
          !$authScreen.classList.contains("hidden") &&
          document.activeElement &&
          (document.activeElement.id === "authEmail" ||
            document.activeElement.id === "authPassword")
        ) {
          $authSubmitBtn.click();
        }
      });

      // ── DB CONNECTION STATUS ──
      const connectedRef = ref(database, ".info/connected");
      onValue(connectedRef, (snap) => {
        const dbBadgeEl = document.getElementById("dbBadge");
        if (snap.val() === true) {
          dbBadgeEl.innerText = "Connected";
          dbBadgeEl.className = "badge connected";
        } else {
          dbBadgeEl.innerText = "Disconnected";
          dbBadgeEl.className = "badge disconnected";
        }
      });

      // ── PUBLISH HELPERS ──
      // Publishes the latest temperature reading to Firebase for persistence and cross-client syncing.
      window.publishTemperatureToDatabase = async function (
        roomId,
        sensorId,
        value,
      ) {
        const telemetryKey = `${roomId}_${sensorId}`;
        const numericValue = Number(value);
        const publishedAt = Date.now();
        lastPublishedTelemetry.set(telemetryKey, {
          value: numericValue,
          publishedAt,
        });
        try {
          await set(
            ref(database, `rooms/${roomId}/sensors/${sensorId}/temperature`),
            Number(value).toFixed(2),
          );
          await set(
            ref(database, `rooms/${roomId}/sensors/${sensorId}/lastUpdated`),
            publishedAt,
          );
        } catch (e) {
          console.error("FB temp write failed:", e);
        }
      };
      //edit new name of sensors to database
      window.publishSensorNameToDatabase = async function (
        roomId,
        sensorId,
        name,
      ) {
        try {
          await set(
            ref(database, `rooms/${roomId}/sensors/${sensorId}/name`),
            name,
          );
        } catch (e) {
          console.error("FB name write failed:", e);
        }
      };

      // Saves a new event-record entry into Firebase to keep the history log synchronized across devices.
      window.publishEventHistoryToDatabase = async function (eventEntry) {
        try {
          const eventId = `${Date.now()}-${eventEntry.roomId || "room"}-${eventEntry.sensorId || "sensor"}`;
          await set(ref(database, `eventHistory/${eventId}`), eventEntry);
        } catch (e) {
          console.error("FB history write failed:", e);
        }
      };

      function updateSensorCardUI(roomId, sensorId, temp, status) {
        const card = document.querySelector(
          `.sensor-mini-card[data-sensor="${sensorId}"][data-room="${roomId}"]`,
        );
        if (!card) return;
        const tempEl = card.querySelector(".sensor-mini-temp");
        const statusEl = card.querySelector(".sensor-mini-status");

        if (tempEl) {
          tempEl.innerText = formatTemp(temp);
          tempEl.style.color =
            status === "CRITICAL"
              ? "var(--danger)"
              : status === "HIGH_HEAT"
                ? "var(--warning)"
                : "var(--success)";
        }

        if (statusEl) {
          statusEl.className = `sensor-mini-status ${
            status === "CRITICAL"
              ? "mini-status-danger"
              : status === "HIGH_HEAT"
                ? "mini-status-warning"
                : "mini-status-normal"
          }`;
          statusEl.innerText = status.replace("_", " ");
        }

        if (status === "CRITICAL") {
          card.classList.add("alarm");
        } else {
          card.classList.remove("alarm");
        }
      }
      // ── LISTEN TO ALL ROOM SENSORS ──
      const roomsRef = ref(database, "rooms");
      onValue(roomsRef, (snapshot) => {
        if (window.ignoreInitialFirebaseTemp) {
          window.ignoreInitialFirebaseTemp = false;
        }
        const data = snapshot.val();
        if (!data) return;

        Object.entries(data).forEach(([roomId, roomData]) => {
          if (!roomData || !roomData.sensors) return;

          Object.entries(roomData.sensors).forEach(([sensorId, sensorData]) => {
            const key = `${roomId}_${sensorId}`;

            // ── Sync sensor name (runs independently of temperature) ──
            const nameFromFB = sensorData?.name;
            if (nameFromFB && typeof nameFromFB === "string") {
              if (
                sensorsData[key] &&
                sensorsData[key].sensorName !== nameFromFB
              ) {
                sensorsData[key].sensorName = nameFromFB;
                // Also update DEFAULT_ROOMS
                const room = DEFAULT_ROOMS.find((r) => r.id === roomId);
                if (room) {
                  const sensor = room.sensors.find((s) => s.id === sensorId);
                  if (sensor) sensor.name = nameFromFB;
                }
                // Re-render UI
                renderSensorCards();
                refreshHeroDisplay();
                // Update thermal map if visible
                const mapPage = document.getElementById("thermalMappingPage");
                if (mapPage && mapPage.style.display !== "none") {
                  populateMapSensorSelect();
                  if (mapState.sensorId && sensorsData[mapState.sensorId]) {
                    loadSensorToMap(mapState.sensorId);
                  }
                }
              }
            }

            // ── Sync temperature (only if present) ──
            if (sensorData.temperature !== undefined) {
              const numericTemp = parseFloat(sensorData.temperature);
              if (isNaN(numericTemp)) return;

              const telemetryKey = key;
              const lastPublished = lastPublishedTelemetry.get(telemetryKey);
              const lastUpdated = Number(sensorData.lastUpdated || 0);

              // Avoid duplicates
              if (
                lastPublished &&
                lastUpdated &&
                lastPublished.publishedAt - lastUpdated <= 1000
              ) {
                lastPublishedTelemetry.delete(telemetryKey);
                return;
              }
              if (
                lastPublished &&
                Math.abs(lastPublished.value - numericTemp) < 0.001
              ) {
                lastPublishedTelemetry.delete(telemetryKey);
                return;
              }

              evaluateMetrics(roomId, sensorId, numericTemp);
            }
          });
        });
      });
      // ── HISTORY LOG SYNC ──
      const eventHistoryRef = ref(database, "eventHistory");
      onValue(eventHistoryRef, (snapshot) => {
        const data = snapshot.val();
        if (!data) {
          globalEventHistory.length = 0;
          renderTableLog(); // ✅ Use the existing function
          return;
        }

        const entries = Object.values(data).sort(
          (a, b) => (b.timestamp || 0) - (a.timestamp || 0),
        );

        const isDifferent =
          entries.length !== globalEventHistory.length ||
          (entries.length > 0 &&
            entries[0].timestamp !== globalEventHistory[0]?.timestamp);

        if (isDifferent) {
          globalEventHistory.length = 0;
          globalEventHistory.push(...entries);
          renderTableLog(); // ✅ Correct function name
        }
      });
      onAuthStateChanged(auth, (user) => {
        if (user) {
          // User is signed in
          window.showDashboard();
          window.setUserBadge(
            user.displayName || user.email || "Authenticated",
          );
          // Store email in localStorage for convenience
          localStorage.setItem("authEmail", user.email);
        } else {
          // User is signed out
          window.showAuthScreen();
          localStorage.removeItem("authEmail");
        }
      });

      // ── SIGN OUT ──
      document
        .getElementById("signOutBtn")
        .addEventListener("click", async () => {
          try {
            await signOut(auth);
            stopSiren();
            $authEmail.value = "";
            $authPassword.value = "";
            $authError.innerText = "";
            $authSuccess.innerText = "";
          } catch (e) {
            console.error("Sign out error:", e);
          }
        });

      // Initial state: show auth screen until Firebase confirms
      window.showAuthScreen();
