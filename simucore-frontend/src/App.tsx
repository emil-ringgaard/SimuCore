import React, { useState, useCallback, useMemo } from 'react';
import { Activity, AlertTriangle, CheckCircle, XCircle, Zap, Settings, Thermometer, RotateCcw, Shield } from 'lucide-react';

// Type definitions
type IOStatus = 'active' | 'warning' | 'error' | 'inactive';
type ComponentType = 'system' | 'controller' | 'safety' | 'regulator' | 'monitor' | 'algorithm';

interface IOPort {
  id: string;
  name: string;
  status: IOStatus;
  value: string;
  unit: string;
}

interface SystemComponent {
  id: string;
  name: string;
  status: IOStatus;
  type: ComponentType;
  inputs: IOPort[];
  outputs: IOPort[];
  components: SystemComponent[];
}

interface SystemStats {
  total: number;
  active: number;
  warning: number;
  error: number;
}

interface IODetailsModalProps {
  io: IOPort | null;
  type: 'input' | 'output' | null;
  onClose: () => void;
}

// Status Icon Component
const StatusIcon: React.FC<{ status: IOStatus; size?: number }> = ({ status, size = 16 }) => {
  switch (status) {
    case 'active':
      return <CheckCircle size={size} className="text-green-500" />;
    case 'warning':
      return <AlertTriangle size={size} className="text-yellow-500" />;
    case 'error':
      return <XCircle size={size} className="text-red-500" />;
    default:
      return <Activity size={size} className="text-gray-400" />;
  }
};

const getStatusColor = (status: IOStatus): string => {
  switch (status) {
    case 'active':
      return 'border-green-500 bg-green-50';
    case 'warning':
      return 'border-yellow-500 bg-yellow-50';
    case 'error':
      return 'border-red-500 bg-red-50';
    default:
      return 'border-gray-300 bg-gray-50';
  }
};

const getComponentIcon = (type: ComponentType): JSX.Element => {
  switch (type) {
    case 'controller':
      return <Settings size={20} className="text-blue-600" />;
    case 'safety':
      return <Shield size={20} className="text-red-600" />;
    case 'regulator':
      return <RotateCcw size={20} className="text-purple-600" />;
    case 'monitor':
      return <Activity size={20} className="text-green-600" />;
    case 'algorithm':
      return <Thermometer size={20} className="text-orange-600" />;
    default:
      return <Settings size={20} className="text-gray-600" />;
  }
};

// Simple Flow Canvas Component (React Flow alternative)
interface FlowNodeProps {
  component: SystemComponent;
  position: { x: number; y: number };
  onSelectIO: (io: IOPort, type: 'input' | 'output') => void;
  onExpandComponent: (component: SystemComponent) => void;
  connections: Array<{ from: string; to: string }>;
}

const FlowNode: React.FC<FlowNodeProps> = ({ 
  component, 
  position, 
  onSelectIO, 
  onExpandComponent,
  connections 
}) => {
  const [isDragging, setIsDragging] = useState(false);
  const [dragOffset, setDragOffset] = useState({ x: 0, y: 0 });
  const [nodePosition, setNodePosition] = useState(position);

  const handleMouseDown = (e: React.MouseEvent) => {
    const rect = e.currentTarget.getBoundingClientRect();
    setDragOffset({
      x: e.clientX - rect.left,
      y: e.clientY - rect.top
    });
    setIsDragging(true);
  };

  const handleMouseMove = useCallback((e: MouseEvent) => {
    if (isDragging) {
      setNodePosition({
        x: e.clientX - dragOffset.x,
        y: e.clientY - dragOffset.y
      });
    }
  }, [isDragging, dragOffset]);

  const handleMouseUp = useCallback(() => {
    setIsDragging(false);
  }, []);

  React.useEffect(() => {
    if (isDragging) {
      document.addEventListener('mousemove', handleMouseMove);
      document.addEventListener('mouseup', handleMouseUp);
      return () => {
        document.removeEventListener('mousemove', handleMouseMove);
        document.removeEventListener('mouseup', handleMouseUp);
      };
    }
  }, [isDragging, handleMouseMove, handleMouseUp]);

  return (
    <div
      className={`absolute p-4 rounded-lg border-2 shadow-lg min-w-[250px] cursor-move ${getStatusColor(component.status)} ${
        isDragging ? 'z-50' : 'z-10'
      }`}
      style={{
        left: nodePosition.x,
        top: nodePosition.y,
        transform: isDragging ? 'scale(1.02)' : 'scale(1)',
        transition: isDragging ? 'none' : 'transform 0.2s ease'
      }}
      onMouseDown={handleMouseDown}
    >
      {/* Connection ports */}
      <div className="absolute left-0 top-8 flex flex-col gap-2">
        {component.inputs?.map((input, index) => (
          <div
            key={input.id}
            className="w-3 h-3 bg-blue-500 rounded-full -ml-1.5 border-2 border-white shadow-md"
            title={`Input: ${input.name}`}
          />
        ))}
      </div>
      
      <div className="absolute right-0 top-8 flex flex-col gap-2">
        {component.outputs?.map((output, index) => (
          <div
            key={output.id}
            className="w-3 h-3 bg-orange-500 rounded-full -mr-1.5 border-2 border-white shadow-md"
            title={`Output: ${output.name}`}
          />
        ))}
      </div>

      <div className="flex items-center gap-2 mb-3">
        {getComponentIcon(component.type)}
        <span className="font-semibold">{component.name}</span>
        <StatusIcon status={component.status} size={16} />
      </div>

      <div className="text-xs text-gray-600 mb-3">
        Type: {component.type}
      </div>

      {/* Inputs */}
      {component.inputs && component.inputs.length > 0 && (
        <div className="mb-3">
          <div className="text-xs font-medium text-blue-700 mb-1 flex items-center gap-1">
            <Zap size={12} className="text-blue-600" />
            Inputs ({component.inputs.length}):
          </div>
          {component.inputs.map((input: IOPort) => (
            <div 
              key={input.id}
              className="text-xs p-2 mb-1 bg-white rounded cursor-pointer hover:bg-blue-50 border"
              onClick={(e) => {
                e.stopPropagation();
                onSelectIO(input, 'input');
              }}
            >
              <div className="flex items-center gap-1">
                <StatusIcon status={input.status} size={10} />
                <span className="font-medium">{input.name}</span>
              </div>
              <div className="text-gray-600">{input.value} {input.unit}</div>
            </div>
          ))}
        </div>
      )}

      {/* Outputs */}
      {component.outputs && component.outputs.length > 0 && (
        <div className="mb-3">
          <div className="text-xs font-medium text-orange-700 mb-1 flex items-center gap-1">
            <Zap size={12} className="text-orange-600 rotate-180" />
            Outputs ({component.outputs.length}):
          </div>
          {component.outputs.map((output: IOPort) => (
            <div 
              key={output.id}
              className="text-xs p-2 mb-1 bg-white rounded cursor-pointer hover:bg-orange-50 border"
              onClick={(e) => {
                e.stopPropagation();
                onSelectIO(output, 'output');
              }}
            >
              <div className="flex items-center gap-1">
                <StatusIcon status={output.status} size={10} />
                <span className="font-medium">{output.name}</span>
              </div>
              <div className="text-gray-600">{output.value} {output.unit}</div>
            </div>
          ))}
        </div>
      )}

      {/* Sub-components indicator */}
      {component.components && component.components.length > 0 && (
        <button 
          className="text-xs bg-white px-3 py-1 rounded border hover:bg-gray-50 flex items-center gap-1"
          onClick={(e) => {
            e.stopPropagation();
            onExpandComponent(component);
          }}
        >
          <Settings size={12} />
          {component.components.length} Sub-components
        </button>
      )}
    </div>
  );
};

// Connection lines component
const ConnectionLines: React.FC<{ connections: Array<{ from: string; to: string; fromPos: { x: number; y: number }; toPos: { x: number; y: number } }> }> = ({ connections }) => {
  return (
    <svg className="absolute inset-0 pointer-events-none" style={{ zIndex: 1 }}>
      {connections.map((connection, index) => {
        const { fromPos, toPos } = connection;
        const midX = (fromPos.x + toPos.x) / 2;
        return (
          <g key={index}>
            <path
              d={`M ${fromPos.x} ${fromPos.y} Q ${midX} ${fromPos.y} ${midX} ${(fromPos.y + toPos.y) / 2} Q ${midX} ${toPos.y} ${toPos.x} ${toPos.y}`}
              stroke="#f97316"
              strokeWidth="2"
              fill="none"
              markerEnd="url(#arrowhead)"
            />
          </g>
        );
      })}
      <defs>
        <marker
          id="arrowhead"
          markerWidth="10"
          markerHeight="7"
          refX="10"
          refY="3.5"
          orient="auto"
        >
          <polygon
            points="0 0, 10 3.5, 0 7"
            fill="#f97316"
          />
        </marker>
      </defs>
    </svg>
  );
};

const IODetailsModal: React.FC<IODetailsModalProps> = ({ io, type, onClose }) => {
  if (!io) return null;

  return (
    <div className="fixed inset-0 bg-black bg-opacity-50 flex items-center justify-center z-50" onClick={onClose}>
      <div className="bg-white rounded-lg p-6 max-w-md w-full mx-4" onClick={(e) => e.stopPropagation()}>
        <div className="flex items-center justify-between mb-4">
          <h2 className="text-xl font-bold flex items-center gap-2">
            {type === 'input' ? <Zap className="text-blue-600" /> : <Zap className="text-orange-600 rotate-180" />}
            {io.name}
          </h2>
          <button 
            onClick={onClose}
            className="text-gray-500 hover:text-gray-700 text-2xl"
          >
            ×
          </button>
        </div>
        
        <div className="space-y-3">
          <div className="flex items-center gap-2">
            <span className="font-medium">Status:</span>
            <StatusIcon status={io.status} />
            <span className="capitalize">{io.status}</span>
          </div>
          
          <div>
            <span className="font-medium">Current Value:</span>
            <div className="text-2xl font-mono mt-1">
              {io.value} {io.unit}
            </div>
          </div>
          
          <div>
            <span className="font-medium">Type:</span>
            <span className="ml-2 capitalize">{type}</span>
          </div>
          
          <div>
            <span className="font-medium">ID:</span>
            <span className="ml-2 font-mono text-sm">{io.id}</span>
          </div>

          <div className="border-t pt-3 mt-4">
            <h3 className="font-medium mb-2">Technical Details</h3>
            <div className="text-sm text-gray-600 space-y-1">
              <div>Update Rate: 100ms</div>
              <div>Data Type: {type === 'input' ? 'Analog' : 'Digital'}</div>
              <div>Last Updated: {new Date().toLocaleTimeString()}</div>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
};

const SCADADashboard: React.FC = () => {
  const [selectedIO, setSelectedIO] = useState<IOPort | null>(null);
  const [selectedIOType, setSelectedIOType] = useState<'input' | 'output' | null>(null);
  const [currentLevel, setCurrentLevel] = useState<string>('main');

  const handleIOSelect = useCallback((io: IOPort, type: 'input' | 'output') => {
    setSelectedIO(io);
    setSelectedIOType(type);
  }, []);

  const closeModal = useCallback(() => {
    setSelectedIO(null);
    setSelectedIOType(null);
  }, []);

  const handleExpandComponent = useCallback((component: SystemComponent) => {
    setCurrentLevel(component.id);
  }, []);

  // Sample system data
  const systemData: SystemComponent = {
    id: 'main_system',
    name: 'Main Control System',
    status: 'active',
    type: 'system',
    inputs: [
      { id: 'sys_in_1', name: 'Power Input', status: 'active', value: '24V DC', unit: 'V' },
      { id: 'sys_in_2', name: 'Network', status: 'active', value: 'Connected', unit: '' }
    ],
    outputs: [
      { id: 'sys_out_1', name: 'Status LED', status: 'active', value: 'Green', unit: '' },
      { id: 'sys_out_2', name: 'Alarm Output', status: 'inactive', value: 'Off', unit: '' }
    ],
    components: [
      {
        id: 'motor_ctrl',
        name: 'Motor Controller',
        status: 'active',
        type: 'controller',
        inputs: [
          { id: 'mc_in_1', name: 'Speed Command', status: 'active', value: '1500', unit: 'RPM' },
          { id: 'mc_in_2', name: 'Enable Signal', status: 'active', value: 'High', unit: '' }
        ],
        outputs: [
          { id: 'mc_out_1', name: 'Motor Speed', status: 'active', value: '1485', unit: 'RPM' },
          { id: 'mc_out_2', name: 'Current Draw', status: 'warning', value: '8.5', unit: 'A' }
        ],
        components: [
          {
            id: 'speed_reg',
            name: 'Speed Regulator',
            status: 'active',
            type: 'regulator',
            inputs: [
              { id: 'sr_in_1', name: 'Setpoint', status: 'active', value: '1500', unit: 'RPM' },
              { id: 'sr_in_2', name: 'Feedback', status: 'active', value: '1485', unit: 'RPM' }
            ],
            outputs: [
              { id: 'sr_out_1', name: 'Control Signal', status: 'active', value: '75', unit: '%' }
            ],
            components: []
          },
          {
            id: 'current_mon',
            name: 'Current Monitor',
            status: 'warning',
            type: 'monitor',
            inputs: [
              { id: 'cm_in_1', name: 'Current Sensor', status: 'active', value: '8.5', unit: 'A' }
            ],
            outputs: [
              { id: 'cm_out_1', name: 'Overload Alert', status: 'warning', value: 'High', unit: '' }
            ],
            components: []
          }
        ]
      },
      {
        id: 'temp_ctrl',
        name: 'Temperature Control',
        status: 'active',
        type: 'controller',
        inputs: [
          { id: 'tc_in_1', name: 'Temperature Sensor', status: 'active', value: '65.2', unit: '°C' },
          { id: 'tc_in_2', name: 'Setpoint', status: 'active', value: '70.0', unit: '°C' }
        ],
        outputs: [
          { id: 'tc_out_1', name: 'Heater Control', status: 'active', value: '45', unit: '%' },
          { id: 'tc_out_2', name: 'Fan Control', status: 'inactive', value: '0', unit: '%' }
        ],
        components: [
          {
            id: 'pid_ctrl',
            name: 'PID Controller',
            status: 'active',
            type: 'algorithm',
            inputs: [
              { id: 'pid_in_1', name: 'Error Signal', status: 'active', value: '4.8', unit: '°C' }
            ],
            outputs: [
              { id: 'pid_out_1', name: 'Output Signal', status: 'active', value: '45', unit: '%' }
            ],
            components: []
          }
        ]
      },
      {
        id: 'safety_sys',
        name: 'Safety System',
        status: 'error',
        type: 'safety',
        inputs: [
          { id: 'ss_in_1', name: 'Emergency Stop', status: 'active', value: 'Released', unit: '' },
          { id: 'ss_in_2', name: 'Door Sensor', status: 'error', value: 'Open', unit: '' }
        ],
        outputs: [
          { id: 'ss_out_1', name: 'Safety Relay', status: 'error', value: 'Tripped', unit: '' },
          { id: 'ss_out_2', name: 'Warning Light', status: 'active', value: 'Flashing', unit: '' }
        ],
        components: []
      }
    ]
  };

  const getCurrentComponents = (): SystemComponent[] => {
    if (currentLevel === 'main') {
      return systemData.components;
    } else {
      const parent = systemData.components.find(c => c.id === currentLevel);
      return parent?.components || [];
    }
  };

  const getSystemStats = useCallback((): SystemStats => {
    const getAllComponents = (component: SystemComponent): SystemComponent[] => {
      let components: SystemComponent[] = [component];
      if (component.components) {
        component.components.forEach(sub => {
          components = components.concat(getAllComponents(sub));
        });
      }
      return components;
    };

    const allComponents = getAllComponents(systemData);
    const active = allComponents.filter(c => c.status === 'active').length;
    const warning = allComponents.filter(c => c.status === 'warning').length;
    const error = allComponents.filter(c => c.status === 'error').length;

    return { total: allComponents.length, active, warning, error };
  }, [systemData]);

  const stats = getSystemStats();
  const currentComponents = getCurrentComponents();

  // Generate node positions and connections
  const nodePositions = useMemo(() => {
    const positions: Array<{ component: SystemComponent; position: { x: number; y: number } }> = [];
    const connections: Array<{ from: string; to: string; fromPos: { x: number; y: number }; toPos: { x: number; y: number } }> = [];

    currentComponents.forEach((component, index) => {
      const x = 100 + (index % 3) * 350;
      const y = 100 + Math.floor(index / 3) * 300;
      positions.push({ component, position: { x, y } });
    });

    // Generate simple connections between components
    if (currentComponents.length > 1) {
      for (let i = 0; i < currentComponents.length - 1; i++) {
        const fromPos = positions[i];
        const toPos = positions[i + 1];
        if (fromPos && toPos) {
          connections.push({
            from: fromPos.component.id,
            to: toPos.component.id,
            fromPos: { x: fromPos.position.x + 250, y: fromPos.position.y + 60 },
            toPos: { x: toPos.position.x, y: toPos.position.y + 60 }
          });
        }
      }
    }

    return { positions, connections };
  }, [currentComponents]);

  return (
    <div className="h-screen bg-gray-100 flex flex-col">
      {/* Header */}
      <div className="bg-white shadow-md p-4 z-20 relative">
        <div className="flex items-center justify-between">
          <div className="flex items-center gap-4">
            <h1 className="text-2xl font-bold text-gray-800">SCADA System Dashboard</h1>
            <div className="flex gap-2">
              {currentLevel !== 'main' && (
                <button 
                  className="px-3 py-1 bg-blue-500 text-white rounded hover:bg-blue-600"
                  onClick={() => setCurrentLevel('main')}
                >
                  ← Back to Main
                </button>
              )}
            </div>
          </div>
          <div className="flex gap-4">
            <div className="text-center">
              <div className="text-xl font-bold text-green-600">{stats.active}</div>
              <div className="text-xs text-gray-600">Active</div>
            </div>
            <div className="text-center">
              <div className="text-xl font-bold text-yellow-600">{stats.warning}</div>
              <div className="text-xs text-gray-600">Warning</div>
            </div>
            <div className="text-center">
              <div className="text-xl font-bold text-red-600">{stats.error}</div>
              <div className="text-xs text-gray-600">Error</div>
            </div>
          </div>
        </div>
      </div>

      {/* Flow Canvas */}
      <div className="flex-1 relative overflow-hidden bg-gray-50" style={{ backgroundImage: 'radial-gradient(circle, #cbd5e1 1px, transparent 1px)', backgroundSize: '20px 20px' }}>
        <ConnectionLines connections={nodePositions.connections} />
        
        {nodePositions.positions.map(({ component, position }) => (
          <FlowNode
            key={component.id}
            component={component}
            position={position}
            onSelectIO={handleIOSelect}
            onExpandComponent={handleExpandComponent}
            connections={nodePositions.connections}
          />
        ))}

        {currentComponents.length === 0 && (
          <div className="absolute inset-0 flex items-center justify-center">
            <div className="text-gray-500 text-lg">No components at this level</div>
          </div>
        )}
      </div>

      {/* Instructions */}
      <div className="bg-blue-50 border-t border-blue-200 p-3 z-20 relative">
        <div className="text-sm text-blue-700 flex flex-wrap gap-4">
          <span>• Click on I/O items for detailed information</span>
          <span>• Click "Sub-components" to drill down into components</span>
          <span>• Drag components to reorganize the layout</span>
          <span>• Use the back button to return to parent level</span>
        </div>
      </div>

      {/* IO Details Modal */}
      <IODetailsModal 
        io={selectedIO} 
        type={selectedIOType} 
        onClose={closeModal} 
      />
    </div>
  );
};

export default SCADADashboard;