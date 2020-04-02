import EmbeddingWindow from './embeddingWindow';
import MorseSmaleWindow from './morseSmaleWindow';
import React from 'react';
import ReactDOM from 'react-dom';
import { withDSXContext } from '../dsxContext.js';

/**
 * Creates windows that displays the 2D Graph Embedding of the data
 * and the Morse-Smale decomposition
 */

const styles = {
  MainContainer: {
    display: 'flex',
    flexDirection: 'column',
  },
  TopContainer: {
    display: 'flex',
    flexDirection: 'row',
    height: '75%',
  },
  VerticalPanel: {
    width: '50%',
    backgroundColor: 'white',
  },
  VerticalDivider: {
    width: '5px',
    backgroundColor: 'grey',
    cursor: 'col-resize',
  },
  BottomContainer: {
    display: 'flex',
    flexDirection: 'column',
    height: '25%',
  },
  HorizontalPanel: {
    width: '100%',
    height: '100%',
    backgroundColor: 'white',
  },
  HorizontalDivider: {
    width: '100%',
    height: '4px',
    backgroundColor: 'grey',
    cursor: 'row-resize',
  },
};

class EmbeddingMorseSmaleWindow extends React.Component {
  /**
   * Creates EmbeddingMorseSmaleWindow object
   * @param {object} props
   */
  constructor(props) {
    super(props);

    this.state = {
      embeddingDimensions: null,
      drawerImages: [],
      isDragging: false,
      isWidth: false,
    };

    this.client = this.props.dsxContext.client;

    this.computeNewSamplesUsingShapeoddsModel = this.computeNewSamplesUsingShapeoddsModel.bind(this);
    this.handleHeightResizeStart = this.handleHeightResizeStart.bind(this);
    this.handleWidthResizeStart = this.handleWidthResizeStart.bind(this);
    this.handleResize = this.handleResize.bind(this);
    this.handleStopResize = this.handleStopResize.bind(this);
  }

  componentDidMount() {
    ReactDOM.findDOMNode(this).addEventListener('mousemove', this.handleResize);
    ReactDOM.findDOMNode(this).addEventListener('mouseup', this.handleStopResize);
    ReactDOM.findDOMNode(this).addEventListener('mouseleave', this.handleStopResize);
  }

  handleWidthResizeStart(event) {
    this.setState({
      isDragging: true,
      isWidth: true,
      initialPos: event.clientX,
    });
  }

  handleHeightResizeStart(event) {
    this.setState({
      isDragging: true,
      isWidth: false,
      initialPos: event.clientY,
    });
  }

  handleResize(event) {
    const { isDragging, isWidth } = this.state;
    let delta = 0;
    if (isDragging ) {
      if (isWidth) {
        delta = event.clientX - this.state.initialPos;
      } else {
        delta = event.clientY - this.state.initialPos;
      }
      console.log(delta);
      this.setState({ delta:delta });
    }
  }

  handleStopResize() {
    this.setState({
      isDragging: false,
      isWidth: false,
    });
  }

  /**
   * Computes new samples using shapeodds model
   * @param {number} datasetId
   * @param {string} category
   * @param {string} fieldname
   * @param {number} persistenceLevel
   * @param {number} crystalID
   * @param {number} numSamples
   * @param {bool} showOrig
   */
  computeNewSamplesUsingShapeoddsModel(datasetId, category, fieldname, persistenceLevel,
    crystalID, numSamples, showOrig) {
    console.log('computeNewSamplesUsingShapeoddsModel('+datasetId+','+fieldname+','+persistenceLevel+','
                +crystalID+','+numSamples+','+showOrig+')');

    // Ask server to compute the N new images for this crystal and add them to the drawer
    this.client.fetchNImagesForCrystal_Shapeodds(datasetId, category, fieldname, persistenceLevel,
      crystalID, numSamples, showOrig)
      .then((result) => {
        const thumbnails = result.thumbnails.map((thumbnail, i) => {
          return {
            img: thumbnail,
            id: i,
          };
        });
        this.setState({ drawerImages:thumbnails });
        // eslint-disable-next-line max-len
        console.log('computeNewSamplesUsingShapeoddsModel returned ' + result.thumbnails.length + ' images; msg: ' + result.msg);
      });
  }

  /**
   * Renders EmbeddingMorseSmaleWindow to screen
   * @return {JSX}
   */
  render() {
    const width = { ...styles.VerticalPanel, width:'40%' };
    return (
      <div style={styles.MainContainer}>
        <div style={styles.TopContainer}>
          <div style={styles.VerticalPanel}>
            <EmbeddingWindow
              dataset={this.props.dataset}
              decomposition={this.props.decomposition}
              embedding={this.props.embedding}
              selectedDesigns={this.props.selectedDesigns}
              onDesignSelection={this.props.onDesignSelection}
              activeDesigns={this.props.activeDesigns}
              numberOfWindows={this.props.numberOfWindows}/>
          </div>
          <div style={styles.VerticalDivider} onMouseDown={(e) => this.handleWidthResizeStart(e)}/>
          <div style={styles.VerticalPanel}>
            <MorseSmaleWindow
              dataset={this.props.dataset}
              decomposition={this.props.decomposition}
              numberOfWindows={this.props.numberOfWindows}
              onCrystalSelection={this.props.onCrystalSelection}
              evalShapeoddsModelForCrystal={this.computeNewSamplesUsingShapeoddsModel}/>
          </div>
        </div>
        <div style={styles.BottomContainer}>
          <div style={styles.HorizontalDivider} onMouseDown={() => console.log('Horizontal Divider Selected')}/>
          <div style={styles.HorizontalPanel}>Will be Drawer!!</div>
        </div>
      </div>);
  }
}

export default withDSXContext(EmbeddingMorseSmaleWindow);
