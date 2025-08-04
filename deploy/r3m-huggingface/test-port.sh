#!/bin/bash

echo "Testing port configuration..."

# Check if the application respects command line arguments
echo "Testing with port 7860..."
./r3m --port 7860 --host 0.0.0.0 &
PID=$!
sleep 5

# Check if it's listening on the correct port
if netstat -tlnp 2>/dev/null | grep :7860; then
    echo "✅ Application is listening on port 7860"
    curl -f http://localhost:7860/health && echo "✅ Health check passed"
else
    echo "❌ Application is not listening on port 7860"
    netstat -tlnp 2>/dev/null | grep :8080 && echo "⚠️  Application is listening on port 8080 instead"
fi

kill $PID 2>/dev/null 