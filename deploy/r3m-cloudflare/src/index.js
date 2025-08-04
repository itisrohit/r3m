import { Container } from "@cloudflare/containers";

/**
 * R3M Container class for running the C++ Docker backend
 */
export class R3MContainer extends Container {
  // The port your C++ container listens on
  defaultPort = 8080;
  // Put the container to sleep after 30s of inactivity
  sleepAfter = "30s";
  // Pass environment variables to the container
  envVars = {
    DOCKER_CONTAINER: "true",
    R3M_ENABLE_MEMORY_POOLING: "false",
    R3M_MAX_WORKERS: "4",
    R3M_OPTIMAL_BATCH_SIZE: "100"
  };

  onStart() {
    console.log("R3M C++ Container started successfully");
  }
}

/**
 * Main Worker that routes requests to the C++ container
 */
export default {
  async fetch(request, env, ctx) {
    // Handle CORS
    if (request.method === 'OPTIONS') {
      return new Response(null, {
        status: 200,
        headers: {
          'Access-Control-Allow-Origin': '*',
          'Access-Control-Allow-Methods': 'GET, POST, PUT, DELETE, OPTIONS',
          'Access-Control-Allow-Headers': 'Content-Type, Authorization',
        },
      });
    }

    const url = new URL(request.url);
    const path = url.pathname;

    // Add CORS headers to all responses
    const corsHeaders = {
      'Access-Control-Allow-Origin': '*',
      'Access-Control-Allow-Methods': 'GET, POST, PUT, DELETE, OPTIONS',
      'Access-Control-Allow-Headers': 'Content-Type, Authorization',
    };

    try {
      // Route all API requests to the C++ container
      if (path.startsWith('/health') || path.startsWith('/info') || 
          path.startsWith('/process') || path.startsWith('/batch') || 
          path.startsWith('/api/')) {
        
        // Get a container instance (load balanced)
        const containerId = Math.floor(Math.random() * 3);
        const id = env.R3M_CONTAINER.idFromName(containerId.toString());
        const containerInstance = env.R3M_CONTAINER.get(id);
        
        // Forward the request to the C++ container
        return containerInstance.fetch(request);
      }

      // Root endpoint - return API info
      if (path === '/' && request.method === 'GET') {
        return new Response(JSON.stringify({
          name: 'R3M Document Processing API',
          version: 'v1',
          description: 'High-performance C++ document processing system deployed on Cloudflare Containers',
          endpoints: {
            '/health': 'Health check',
            '/info': 'API information', 
            '/process': 'Process single document',
            '/batch': 'Process batch of documents'
          },
          deployment: 'Cloudflare Containers (C++ Docker)'
        }), {
          status: 200,
          headers: {
            'Content-Type': 'application/json',
            ...corsHeaders
          }
        });
      }

      // 404 for unknown endpoints
      return new Response(JSON.stringify({
        error: 'Endpoint not found',
        message: `The endpoint ${path} does not exist`,
        available_endpoints: ['/', '/health', '/info', '/process', '/batch']
      }), {
        status: 404,
        headers: {
          'Content-Type': 'application/json',
          ...corsHeaders
        }
      });

    } catch (error) {
      return new Response(JSON.stringify({
        error: 'Internal server error',
        message: error.message
      }), {
        status: 500,
        headers: {
          'Content-Type': 'application/json',
          ...corsHeaders
        }
      });
    }
  },
}; 