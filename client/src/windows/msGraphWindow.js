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
          numberOfWindows={this.props.numberOfWindows}/>
        <MorseSmaleWindow/>
      </div>
    );
  }
}

export default MsGraphWindow;