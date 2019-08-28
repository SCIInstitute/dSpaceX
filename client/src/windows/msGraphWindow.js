import GraphGLWindow from './graphGLWindow';
import MorseSmaleWindow from './morseSmaleWindow';
import React from 'react';

class MsGraphWindow extends React.Component {
  constructor(props) {
    super(props);
  }

  render() {
    let container = {
      display: 'flex',
      flexDirection: 'row',
    };
    return (
      <div style={container}>
        <GraphGLWindow
          dataset={this.props.dataset}
          decomposition={this.props.decomposition}
          selectedDesigns={this.props.selectedDesigns}
          onDesignSelection={this.props.onDesignSelection}
          activeDesigns={this.props.activeDesigns}
          numberOfWindows={this.props.numberOfWindows}/>
        <MorseSmaleWindow
          dataset={this.props.dataset}
          decomposition={this.props.decomposition}
          numberOfWindows={this.props.numberOfWindows}/>
      </div>
    );
  }
}

export default MsGraphWindow;