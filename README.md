# MICRO_ROS_ETH

## Architecture (Mermaid)

```mermaid
flowchart LR
  MCU[MCU (micro-ROS node)] -->|Ethernet| AGENT[micro-ROS Agent]
  AGENT --> ROS2[ROS 2 graph]
```
