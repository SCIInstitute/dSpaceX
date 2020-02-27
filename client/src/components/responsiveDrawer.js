import React, { Component } from 'react';
import { Drawer } from '@material-ui/core';
import Grid from '@material-ui/core/Grid';
import Paper from '@material-ui/core/Paper';
import withStyles from '@material-ui/core/es/styles/withStyles';

const styles = () => ({
  dragger: {
    height: '100%',
    width: '100%',
    cursor: 'n-resize',
    padding: '5px 5px 5px 5px',
    borderTop: '1px solid #ddd',
    position: 'absolute',
    top: 0,
    bottoms: 0,
    // zIndex: '100',
    backgroundColor: '#f4f7f9',
  },
});

/**
 * This class create a drawer that user can resize
 */
class ResponsiveDrawer extends Component {
  /**
   * Create ResponsiveDrawer object
   * @param {object} props
   */
  constructor(props) {
    super(props);
    this.state = {
      isResizing: false,
      lastDownY: 0,
      newHeight: { height:150 },
    };

    this.handleMouseDown = this.handleMouseDown.bind(this);
    this.handleMouseMove = this.handleMouseMove.bind(this);
    this.handleMouseUp = this.handleMouseUp.bind(this);
  }

  /**
   * Handles mouse down event
   * @param {event} e
   */
  handleMouseDown(e) {
    this.setState({ isResizing:true, lastDownY:e.clientY });
  }

  /**
   * Handles mouse move event
   * @param {MouseEvent} e
   */
  handleMouseMove(e) {
    if (!this.state.isResizing) {
      return;
    }

    let offsetBottom = document.body.offsetHeight - (e.clientY - document.body.offsetTop);
    let minHeight = 50;
    let maxHeight = 600;
    if (offsetBottom > minHeight && offsetBottom < maxHeight) {
      this.setState({ newHeight:{ height:offsetBottom } });
    }
  }

  /**
   * Handles mouse up event
   * @param {MouseEvent} e
   */
  handleMouseUp(e) {
    this.setState({ isResizing:false });
  }

  /**
   * React function called when component mounts.
   */
  componentDidMount() {
    document.addEventListener('mousemove', (e) => this.handleMouseMove(e));
    document.addEventListener('mouseup', (e) => this.handleMouseUp(e));
  }

  /**
   * React function renders component
   * @return {JSX}
   */
  render() {
    const { classes } = this.props;
    return (
      <Drawer
        variant="permanent"
        open
        anchor={'bottom'}
        classes={{ paper:classes.drawerPaper }}
        PaperProps={{ style:this.state.newHeight }}>
        <div id='dragger' onMouseDown={(event) => { this.handleMouseDown(event); }} className={classes.dragger}>
          <Grid container
            justify={'flex-end'}
            spacing={8}
            style={{ margin:'5px 0px 0px 0px' }}>
            {this.props.images.length > 0 &&
         this.props.images.map((thumbnail, i) =>
           <Grid key={i} item>
             <Paper style={{ backgroundColor:'#D3D3D3' }}>
               <img alt={'Image:' + thumbnail.id}
                 height='45'
                 style={{ margin:'5px 5px 5px 5px' }}
                 src={'data:image/png;base64, ' + thumbnail.img.rawData}/>
             </Paper>
           </Grid>)}
          </Grid>
        </div>
      </Drawer>
    );
  }
}

export default withStyles(styles, { withTheme:true })(ResponsiveDrawer);
