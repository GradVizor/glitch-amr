# Docker Setup Guide

This guide provides step-by-step instructions for setting up Docker to run the Glitch AMR ROS2 project, including native Docker Engine installation, graphics configuration for Gazebo and RViz2, and container deployment.

---

## Step 1: Install Native Docker Engine (Ubuntu 22.04)

Once the system has restarted, install the native Docker Engine from the official Docker repository.

```bash
# 1. Update package list and install prerequisites
sudo apt-get update
sudo apt-get install -y ca-certificates curl

# 2. Add Docker's official GPG key
sudo install -m 0755 -d /etc/apt/keyrings
sudo curl -fsSL https://download.docker.com/linux/ubuntu/gpg -o /etc/apt/keyrings/docker.asc
sudo chmod a+r /etc/apt/keyrings/docker.asc

# 3. Add the Docker repository to Apt sources
echo \
  "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.asc] https://download.docker.com/linux/ubuntu \
  $(. /etc/os-release && echo "$VERSION_CODENAME") stable" | \
  sudo tee /etc/apt/sources.list.d/docker.list > /dev/null

# 4. Update the package list and install Docker CLI, Engine, and Compose
sudo apt-get update
sudo apt-get install -y docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin
```

### Post-Installation: Enable Non-Root User Access

To run your containers without needing to prepend `sudo` to every Docker command:

```bash
# Create the docker group (if not automatically created)
sudo groupadd docker 2>/dev/null

# Add your current user to the docker group
sudo usermod -aG docker $USER

# Apply the group changes immediately
newgrp docker
```

---

## Step 2: Host Graphics & Permissions Configuration

To display Gazebo, RViz2, or any other Graphical User Interface (GUI) inside the container onto your host screen, you must grant drawing permissions and configure the graphics toolkit.

### 1. Allow X11 Local Connections (Required after every system login)

Instruct your Ubuntu desktop system to accept incoming graphics windows created by local Docker containers:

```bash
xhost +local:docker
```

### 2. Optional: Configure Nvidia GPU Acceleration

If your host computer has a dedicated Nvidia Graphics Card and you plan to run Gazebo utilizing your GPU:

```bash
# Install the Nvidia Container Toolkit on your host OS
sudo apt-get update
sudo apt-get install -y nvidia-container-toolkit

# Restart the Docker service to apply the configuration
sudo systemctl restart docker
```

---

## Step 3: Understanding the Configuration Files

Your project relies on three configuration files located at the root of your repository.

- **Dockerfile**: Installs the underlying operating system (ROS2 Humble Desktop Full), downloads essential robotics packages (Nav2, Cartographer, Slam Toolbox, etc.), fetches the micro-ROS Agent workspace repository, and sets up the internal environment.
- **docker-compose.yml**: Orchestrates the build. It shares host network ports, links system graphics (Intel, AMD, or Nvidia), forwards host-screen authorization keys, and mounts your local source code directory directly into the active container workspace so that any edit you make locally is immediately reflected in your container.
- **entrypoint.sh**: A lightweight initialization script that runs automatically whenever the container is spun up. It ensures that standard ROS2 terminal variables and your compiled packages are systematically sourced inside every new terminal session.

---

## Step 4: Building and Running the Container

Navigate to your workspace directory on your host machine containing these configuration files and execute the build sequence.

```bash
# 1. Build the Docker image (this may take several minutes on the first run)
docker compose build

# 2. Spin up the container in the background (detached mode)
docker compose up -d
```

To verify the container is running:

```bash
docker ps
```

---

## Step 5: Working Inside the ROS2 Workspace

To write and execute your ROS2 programs, interact with the container using an interactive terminal.

### 1. Open an interactive shell inside the container

```bash
docker exec -it glitch_amr_container bash
```

### 2. Compile your ROS2 workspace (Run inside the container)

Once inside the container shell, navigate to the workspace root and build the project packages using colcon:

```bash
cd /ros2_ws
colcon build --symlink-install
```

### 3. Source the built workspace (Run inside the container)

Ensure your newly compiled nodes are recognized by the system:

```bash
source install/setup.bash
```

Now, any launch file, node run command, or simulator (Gazebo/RViz2) you run from this container shell will project its graphics natively onto your monitor.
