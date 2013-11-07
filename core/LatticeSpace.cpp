#include "LatticeSpace.hpp"

namespace ecell4
{

LatticeSpace::LatticeSpace()
    :  theNormalizedVoxelRadius(0.5)
{
    // example
    edge_lengths_[0] = 10;
    edge_lengths_[1] = 5;
    edge_lengths_[2] = 15;

    set_lattice_properties();
}

LatticeSpace::~LatticeSpace()
{
}

Integer LatticeSpace::num_species() const
{
    return spmap_.size();
}

bool LatticeSpace::has_species(const Species& sp) const
{
    return spmap_.find(sp) != spmap_.end();
}

Integer LatticeSpace::num_molecules(const Species& sp) const
{
    return num_particles(sp) * 1;
}

const Position3& LatticeSpace::edge_lengths() const
{
    return edge_lengths_;
}

Integer LatticeSpace::num_particles() const
{
    Integer count(0);
    for (spmap::const_iterator itr(spmap_.begin());
            itr != spmap_.end(); ++itr)
    {
        count += (*itr).second.voxels().size();
    }

    return count;
}

Integer LatticeSpace::num_particles(const Species& sp) const
{
    spmap::const_iterator itr(spmap_.find(sp));
    Integer count(0);
    if (itr != spmap_.end())
    {
        count = (*itr).second.voxels().size();
    }
    return count;
}

bool LatticeSpace::has_particle(const ParticleID& pid) const
{
    bool flg(false);
    for (spmap::const_iterator itr(spmap_.begin());
            itr != spmap_.end(); ++itr)
    {
        const MolecularType& mt((*itr).second);
        const MolecularType::container_type voxels(mt.voxels());
        for (MolecularType::container_type::const_iterator vitr(voxels.begin());
                vitr != voxels.end(); ++vitr)
        {
            if ((*vitr).second == pid)
            {
                flg = true;
                break;
            }
        }
    }

    return flg;
}

std::vector<std::pair<ParticleID, Particle> >
    LatticeSpace::list_particles() const
{
    std::vector<std::pair<ParticleID, Particle> > retval;
    for (spmap::const_iterator itr(spmap_.begin());
            itr != spmap_.end(); ++itr)
    {
        const MolecularType& mt((*itr).second);
        const MolecularType::container_type voxels(mt.voxels());
        for (MolecularType::container_type::const_iterator vitr(voxels.begin());
                vitr != voxels.end(); ++vitr)
        {
            const Voxel *voxel((*vitr).first);
            retval.push_back(std::pair<ParticleID, Particle>(
                        (*vitr).second, voxel2particle(*voxel)));
        }
    }

    return retval;
}

std::vector<std::pair<ParticleID, Particle> >
    LatticeSpace::list_particles(const Species& sp) const
{
    std::vector<std::pair<ParticleID, Particle> > retval;
    spmap::const_iterator itr(spmap_.find(sp));
    if (itr != spmap_.end())
    {
        const MolecularType& mt((*itr).second);
        const MolecularType::container_type voxels(mt.voxels());
        for (MolecularType::container_type::const_iterator vitr(voxels.begin());
                vitr != voxels.end(); ++vitr)
        {
            const Voxel *voxel((*vitr).first);
            retval.push_back(std::pair<ParticleID, Particle>(
                        (*vitr).second, voxel2particle(*voxel)));
        }
    }

    return retval;
}

/*
 * Spatiocyte methods
 */

/*
 * derived from SpatiocyteStepper::setLatticeProperties()
 */
void LatticeSpace::set_lattice_properties()
{
    lattice_type_ = HCP_LATTICE;

    theHCPl = theNormalizedVoxelRadius/sqrt(3);
    theHCPx = theNormalizedVoxelRadius*sqrt(8.0/3); //Lx
    theHCPy = theNormalizedVoxelRadius*sqrt(3); //Ly

    Integer lengthX = edge_lengths_[0];
    Integer lengthY = edge_lengths_[1];
    Integer lengthZ = edge_lengths_[2];

    theCenterPoint[2] = lengthZ / 2 + 4 *
        theNormalizedVoxelRadius; //row
    theCenterPoint[1] = lengthY / 2 + 2 * theHCPy; //layer
    theCenterPoint[0] = lengthX / 2 + 2 * theHCPx; //column

    row_size_ = (Integer)rint((theCenterPoint[2])/
                              (theNormalizedVoxelRadius));
    layer_size_ = (Integer)rint((theCenterPoint[1]*2)/theHCPy);
    col_size_ = (Integer)rint((theCenterPoint[0]*2)/theHCPx);

    for (Integer coord(0); coord < row_size_ * layer_size_ * col_size_; ++coord)
    {
        Voxel voxel(coord, NULL);
        voxels_.push_back(voxel);
    }
    //theNullCoord = row_size_ * layer_size_ * col_size_;
}

/*
 * original methods
 */

bool LatticeSpace::update_sparticle(ParticleID pid, SParticle spcl)
{
    Voxel& dest = voxel_at(spcl.coord);

    MolecularType& dest_mt = get_molecular_type(spcl.species);

    if (has_particle(pid))
    {
        Voxel& src = voxel_as(pid);
        MolecularTypeBase* src_ptr_mt(src.ptr_mt);
        src_ptr_mt->removeVoxel(pid);
        vacant_type_.addVoxel(&src);
    }
    dest_mt.addVoxel(&dest, pid);

    return true;
}

Species LatticeSpace::add_molecular_type(const std::string name)
{
    const MolecularType mt(name);
    spmap_.insert(spmap::value_type(mt.species(), mt));
    return mt.species();
}

MolecularType& LatticeSpace::get_molecular_type(Species& sp)
{
    spmap::iterator itr(spmap_.find(sp));
    if (itr == spmap_.end())
    {
        throw "Exception in get_molerular_type";
    }
    return (*itr).second;
}

Global LatticeSpace::coord2global(Integer aCoord) const
{
    Global retval;
    retval.col = aCoord / (row_size_ * layer_size_);
    retval.layer = (aCoord % (row_size_ * layer_size_)) / row_size_;
    retval.row = (aCoord % (row_size_ * layer_size_)) % row_size_;
    return retval;
}

const Position3 LatticeSpace::coord2position(Integer coord) const
{
    Integer aGlobalCol;
    Integer aGlobalLayer;
    Integer aGlobalRow;
    Global global(coord2global(coord));
    //the center point of a voxel
    Position3 position;
    switch(lattice_type_)
    {
        case HCP_LATTICE:
            position[1] = (global.col % 2) * theHCPl + theHCPy * global.layer;
            position[2] = global.row * 2 * theNormalizedVoxelRadius +
            ((global.layer + global.col) % 2) * theNormalizedVoxelRadius;
            position[0] = global.col * theHCPx;
            break;
        case CUBIC_LATTICE:
            position[1] = global.layer * 2 * theNormalizedVoxelRadius;
            position[2] = global.row * 2 * theNormalizedVoxelRadius;
            position[0] = global.col * 2 * theNormalizedVoxelRadius;
            break;
    }
    return position;
}

/*
* Protected methods
*/
Voxel& LatticeSpace::voxel_as(ParticleID pid)
{
    for (spmap::iterator itr(spmap_.begin());
            itr != spmap_.end(); ++itr)
    {
        MolecularType& mt((*itr).second);
        MolecularType::container_type voxels(mt.voxels());
        for (MolecularType::container_type::iterator vitr(voxels.begin());
                vitr != voxels.end(); ++vitr)
        {
            if ((*vitr).second == pid)
            {
                return *((*vitr).first);
            }
        }
    }
    throw "Exception: Not in lattice";
}

Voxel& LatticeSpace::voxel_at(Integer coord)
{
    return voxels_.at(coord);
}

Integer LatticeSpace::global2coord(const Global& global) const
{
    return global.row +
        row_size_ * global.layer +
        row_size_ * layer_size_ * global.col;
}

}
