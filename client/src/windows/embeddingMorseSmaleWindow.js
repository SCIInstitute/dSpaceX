import EmbeddingWindow from './embeddingWindow';
// import GraphGLWindow from './graphGLWindow';
import MorseSmaleWindow from './morseSmaleWindow';
import React from 'react';
import ResponsiveDrawer from '../components/responsiveDrawer';

/**
 * Creates windows that displays the 2D Graph Embedding of the data
 * and the Morse-Smale decomposition
 */
class EmbeddingMorseSmaleWindow extends React.Component {
  /**
   * Creates EmbeddingMorseSmaleWindow object
   * @param {object} props
   */
  constructor(props) {
    super(props);
  }

  /**
   * Renders EmbeddingMorseSmaleWindow to screen
   * @return {JSX}
   */
  render() {
    let container = {
      display: 'flex',
      flexDirection: 'row',
    };
    return (
      <div style={container}>
        <EmbeddingWindow
          dataset={this.props.dataset}
          decomposition={this.props.decomposition}
          selectedDesigns={this.props.selectedDesigns}
          onDesignSelection={this.props.onDesignSelection}
          activeDesigns={this.props.activeDesigns}
          numberOfWindows={this.props.numberOfWindows}/>
        {/* <GraphGLWindow*/}
        {/*  dataset={this.props.dataset}*/}
        {/*  decomposition={this.props.decomposition}*/}
        {/*  selectedDesigns={this.props.selectedDesigns}*/}
        {/*  onDesignSelection={this.props.onDesignSelection}*/}
        {/*  activeDesigns={this.props.activeDesigns}*/}
        {/*  numberOfWindows={this.props.numberOfWindows}/>*/}
        <MorseSmaleWindow
          dataset={this.props.dataset}
          decomposition={this.props.decomposition}
          numberOfWindows={this.props.numberOfWindows}
          onCrystalSelection={this.props.onCrystalSelection}/>
        <ResponsiveDrawer/>
      </div>
    );
  }
}

export default EmbeddingMorseSmaleWindow;
